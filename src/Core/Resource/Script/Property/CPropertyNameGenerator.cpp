#include "CPropertyNameGenerator.h"
#include "IUIRelay.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include "Core/Resource/Script/NPropertyMap.h"
#include <Common/Hash/CCRC32.h>

#include <memory>
#include <thread>

/** Default constructor */
CPropertyNameGenerator::CPropertyNameGenerator() = default;

void CPropertyNameGenerator::Warmup()
{
    std::unique_lock lock{mWarmupMutex};

    if (mWordListLoadFinished)
    {
        return;
    }
    mWordListLoadFinished = false;
    mWords.clear();

    // Load the word list from the file
    using FILEPtr = std::unique_ptr<FILE, decltype(&std::fclose)>;
    auto pListFile = FILEPtr{std::fopen(*(gDataDir + "resources/WordList.txt"), "r"), std::fclose};
    ASSERT(pListFile);

    while (!feof(pListFile.get()))
    {
        char WordBuffer[64];
        std::fgets(WordBuffer, sizeof(WordBuffer), pListFile.get());
        WordBuffer[0] = TString::CharToUpper(WordBuffer[0]);

        mWords.emplace_back(TString(WordBuffer).Trimmed());
    }

    mWordListLoadFinished = true;
}

void CPropertyNameGenerator::Generate(const SPropertyNameGenerationParameters& rkParams, IProgressNotifier* pProgress)
{
    // Make sure all prerequisite data is loaded!
    ASSERT(!mIsRunning);
    ASSERT(rkParams.TypeNames.size() > 0);
    mGeneratedNames.clear();
    mValidTypePairMap.clear();
    mIsRunning = true;

    // Convert the type pair map.
    // Also, replace the normal type name list with whatever is in the ID pairs list we were given.
    if (!rkParams.ValidIdPairs.empty())
    {
        mTypeNames.clear();

        for (const SPropertyIdTypePair& kPair : rkParams.ValidIdPairs)
        {
            mValidTypePairMap[ kPair.ID ] = kPair.pkType;
            NBasics::VectorAddUnique( mTypeNames, TString(kPair.pkType) );
        }
    }
    else
    {
        mTypeNames = rkParams.TypeNames;
    }

    // If TestIntsAsChoices is enabled, and int is in the type list, then choice must be in the type list too.
    if (rkParams.TestIntsAsChoices && NBasics::VectorFind(mTypeNames, TString("int")) >= 0)
    {
        NBasics::VectorAddUnique(mTypeNames, TString("choice"));
    }

    // If we haven't loaded the word list yet, load it.
    Warmup();

    // Calculate the number of steps involved in this task.
    const size_t kNumWords = mWords.size();
    const int kMaxWords = rkParams.MaxWords;
    TotalTests = 1;

    for (int i = 0; i < kMaxWords; i++)
        TotalTests *= kNumWords;

    pProgress->SetOneShotTask("Generating property names");
    pProgress->Report(0, TotalTests);

    const uint WordsPerThread = kNumWords / rkParams.ConcurrentTasks;
    std::vector<std::thread> Threads;
    for (int i = 0; i < rkParams.ConcurrentTasks; ++i)
    {
        SPropertyNameGenerationTaskParameters Params{};
        Params.TaskIndex = i;
        Params.StartWord = WordsPerThread * i;
        if (i == rkParams.ConcurrentTasks - 1)
        {
            // Ensure last task takes any remaining words
            Params.EndWord = kNumWords - 1;
        }
        else
        {
            Params.EndWord = Params.StartWord + WordsPerThread;
        }
        Threads.emplace_back(&CPropertyNameGenerator::GenerateTask, this, rkParams, Params, pProgress);
    }
    for (auto& Thread : Threads)
    {
        Thread.join();
    }

    mIsRunning = false;
}

void CPropertyNameGenerator::GenerateTask(const SPropertyNameGenerationParameters& rkParams,
                                          SPropertyNameGenerationTaskParameters taskParams,
                                          IProgressNotifier* pProgress)
{
    const size_t kNumWords = mWords.size();
    const int kMaxWords = rkParams.MaxWords;

    // Configure params needed to run the name generation!
    bool WriteToLog = rkParams.PrintToLog;
    bool SaveResults = true;
    uint64 TestsDone = 0;

    // The prefix only needs to be hashed this one time
    CCRC32 PrefixHash;
    PrefixHash.Hash( *rkParams.Prefix );

    // Use a stack to keep track of the current word we are on. We can use this
    // to cache the hash of a word and then re-use it later instead of recaculating
    // the same hashes over and over. Init the stack with the first word.
    struct SWordCache
    {
        uint WordIndex;
        CCRC32 Hash;
    };
    std::vector<SWordCache> WordCache;

    SWordCache FirstWord { taskParams.StartWord - 1, CCRC32() };
    WordCache.push_back(FirstWord);

    while ( true )
    {
        // Increment the current word, handle wrapping back to 0, and update cached hashes as needed.
        int RecalcIndex = WordCache.size() - 1;
        WordCache.back().WordIndex++;

        while (WordCache[RecalcIndex].WordIndex >= kNumWords ||
               (RecalcIndex == 0 && WordCache[0].WordIndex >= taskParams.EndWord))
        {
            if (RecalcIndex == 0)
            {
                WordCache[0].WordIndex = taskParams.StartWord;

                SWordCache NewWord { 0, CCRC32() };
                WordCache.push_back(NewWord);
            }
            else
            {
                WordCache[RecalcIndex].WordIndex = 0;

                RecalcIndex--;
                WordCache[RecalcIndex].WordIndex++;
            }
        }

        // If we've hit the word limit, break out and end the name generation system.
        if (WordCache.size() > kMaxWords)
            break;

        // Now that all words are updated, calculate the new hashes.
        CCRC32 LastValidHash = (RecalcIndex > 0 ? WordCache[RecalcIndex - 1].Hash : PrefixHash);

        for (; RecalcIndex < WordCache.size(); RecalcIndex++)
        {
            int Index = WordCache[RecalcIndex].WordIndex;

            // For camelcase, hash the first letter of the first word as lowercase
            if (RecalcIndex == 0 && rkParams.Casing == ENameCasing::camelCase)
            {
                const char* pkWord = *mWords[Index];
                LastValidHash.Hash( TString::CharToLower( pkWord[0] ) );
                LastValidHash.Hash( &pkWord[1] );
            }
            else
            {
                // Add an underscore for snake case
                if (RecalcIndex > 0 && rkParams.Casing == ENameCasing::Snake_Case)
                    LastValidHash.Hash("_");

                LastValidHash.Hash( *mWords[Index] );
            }

            WordCache[RecalcIndex].Hash = LastValidHash;
        }

        // We got our hash yay! Now hash the suffix and then we can test with each type name
        CCRC32 BaseHash = LastValidHash;
        BaseHash.Hash( *rkParams.Suffix );

        for (const auto& typeName : mTypeNames)
        {
            CCRC32 FullHash = BaseHash;
            const char* pkTypeName = *typeName;
            FullHash.Hash( pkTypeName );
            const uint32 PropertyID = FullHash.Digest();

            // Check if this hash is a property ID
            if (IsValidPropertyID(PropertyID, pkTypeName, rkParams))
            {
                std::unique_lock lock{mPropertyCheckMutex};

                SGeneratedPropertyName PropertyName;
                NPropertyMap::RetrieveXMLsWithProperty(PropertyID, pkTypeName, PropertyName.XmlList);

                // Generate a string with the complete name. (We wait to do this until now to avoid needless string allocation)
                PropertyName.Name = rkParams.Prefix;

                for (size_t WordIdx = 0; WordIdx < WordCache.size(); WordIdx++)
                {
                    const int Index = WordCache[WordIdx].WordIndex;

                    if (WordIdx > 0 && rkParams.Casing == ENameCasing::Snake_Case)
                    {
                        PropertyName.Name += "_";
                    }

                    PropertyName.Name += mWords[Index];
                }

                if (rkParams.Casing == ENameCasing::camelCase)
                {
                    PropertyName.Name[0] = TString::CharToLower( PropertyName.Name[0] );
                }

                PropertyName.Name += rkParams.Suffix;
                PropertyName.Type = pkTypeName;
                PropertyName.ID = PropertyID;

                if (SaveResults)
                {
                    mGeneratedNames.push_back(PropertyName);

                    // Check if we have too many saved results. This can cause memory issues and crashing.
                    // If we have too many saved results, then to avoid crashing we will force enable log output.
                    if (mGeneratedNames.size() > 9999)
                    {
                        gpUIRelay->ShowMessageBoxAsync("Warning", "There are over 10,000 results. Results will no longer print to the screen. Check the log for the remaining output.");
                        WriteToLog = true;
                        SaveResults = false;
                    }
                }

                // Log this out
                if ( WriteToLog )
                {
                    TString DelimitedXmlList;

                    for (const auto& xml : PropertyName.XmlList)
                    {
                        DelimitedXmlList += xml + '\n';
                    }

                    debugf("%s [%s] : 0x%08X\n%s", *PropertyName.Name, *PropertyName.Type, PropertyName.ID, *DelimitedXmlList);
                }
            }
        }

        // Every 5000 tests, check with the progress notifier. Update the progress
        // bar and check whether the user has requested to cancel the operation.
        TestsDone++;

        if ( (TestsDone % 5000) == 0 )
        {
            if (pProgress->ShouldCancel())
                break;

            std::unique_lock lock{mWarmupMutex};
            auto Value = TotalTestsDone += 5000;
            pProgress->Report(Value, TotalTests);
        }
    }
}

/** Returns whether a given property ID is valid */
bool CPropertyNameGenerator::IsValidPropertyID(uint32 ID, const char*& pkType, const SPropertyNameGenerationParameters& rkParams)
{
    if (!mValidTypePairMap.empty())
    {
        auto Find = mValidTypePairMap.find(ID);

        if (Find != mValidTypePairMap.end())
        {
            if (strcmp( Find->second, pkType ) == 0)
            {
                return true;
            }
            else if (rkParams.TestIntsAsChoices && strcmp(pkType, "choice") == 0)
            {
                if (strcmp( Find->second, "int" ) == 0)
                {
                    pkType = "int";
                    return true;
                }
            }

            return false;
        }
        else
            return false;
    }
    else
    {
        bool IsAlreadyNamed;
        bool IsValid = NPropertyMap::IsValidPropertyID(ID, pkType, &IsAlreadyNamed);

        if (!IsValid && rkParams.TestIntsAsChoices && strcmp(pkType, "choice") == 0)
        {
            IsValid = NPropertyMap::IsValidPropertyID(ID, "int", &IsAlreadyNamed);

            if (IsValid)
            {
                pkType = "int";
            }
        }

        return IsValid && (!IsAlreadyNamed || !rkParams.ExcludeAccuratelyNamedProperties);
    }
}
