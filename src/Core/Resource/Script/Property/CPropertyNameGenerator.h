#ifndef CPROPERTYNAMEGENERATOR_H
#define CPROPERTYNAMEGENERATOR_H

#include "Core/IProgressNotifier.h"
#include <Common/Common.h>

#include <atomic>
#include <mutex>

/** Name casing parameter */
enum class ENameCasing
{
    PascalCase,
    Snake_Case,
    camelCase,
};

/** ID/type pairing for ID pool */
struct SPropertyIdTypePair
{
    uint32 ID;
    const char* pkType;
};

/** Parameters for using the name generator */
struct SPropertyNameGenerationParameters
{
    /** Number of concurrent tasks to run */
    int ConcurrentTasks;

    /** Maximum number of words per name; name generation will complete when all possibilities have been checked */
    int MaxWords;

    /** Prefix to include at the beginning of every name */
    TString Prefix;

    /** Suffix to include at the end of every name */
    TString Suffix;

    /** Name casing to use */
    ENameCasing Casing;

    /** List of valid type suffixes */
    std::vector<TString> TypeNames;

    /** List of ID/type pairs to check against. If empty, all properties are valid. */
    std::vector<SPropertyIdTypePair> ValidIdPairs;

    /** Whether to exclude properties that already have accurate names from the generation results. */
    bool ExcludeAccuratelyNamedProperties;

    /** Whether to test int properties as choices */
    bool TestIntsAsChoices;

    /** Whether to print the output from the generation process to the log */
    bool PrintToLog;
};

struct SPropertyNameGenerationTaskParameters
{
    /** Task index */
    uint TaskIndex;

    /** Base word start index */
    uint StartWord;

    /** Base word end index */
    uint EndWord;
};

/** A generated property name */
struct SGeneratedPropertyName
{
    TString Name;
    TString Type;
    uint32 ID;
    std::set<TString> XmlList;
};

/** Generates property names and validates them against know property IDs. */
class CPropertyNameGenerator
{
    /** Whether the word list has been fully loaded */
    std::atomic<bool> mWordListLoadFinished = false;

    /** Whether the generation process is running */
    bool mIsRunning = false;

    /** List of valid property types to check against */
    std::vector<TString> mTypeNames;

    /** Mapping of valid ID/type pairs; if empty, all property names in NPropertyMap are allowed */
    std::unordered_map<uint32, const char*> mValidTypePairMap;

    /** List of words */
    std::vector<TString> mWords;

    /** List of output generated property names */
    std::list<SGeneratedPropertyName> mGeneratedNames;

    /** Warmup() mutex */
    std::mutex mWarmupMutex;

    /** Property check mutex */
    std::mutex mPropertyCheckMutex;

    /** Total number of tests to perform */
    uint64 TotalTests;

    /** Current number of tests performed */
    std::atomic<uint64> TotalTestsDone{0};

    void GenerateTask(const SPropertyNameGenerationParameters& rkParams,
                      SPropertyNameGenerationTaskParameters taskParams,
                      IProgressNotifier* pProgressNotifier);

public:
    /** Default constructor */
    CPropertyNameGenerator();

    /** Prepares the generator for running name generation */
    void Warmup();

    /** Run the name generation system */
    void Generate(const SPropertyNameGenerationParameters& rkParams, IProgressNotifier* pProgressNotifier);

    /** Returns whether a given property ID is valid */
    bool IsValidPropertyID(uint32 ID, const char*& pkType, const SPropertyNameGenerationParameters& rkParams);

    /** Accessors */
    bool IsRunning() const
    {
        return mIsRunning;
    }

    const std::list<SGeneratedPropertyName>& GetOutput() const
    {
        return mGeneratedNames;
    }
};

#endif // CPROPERTYNAMEGENERATOR_H
