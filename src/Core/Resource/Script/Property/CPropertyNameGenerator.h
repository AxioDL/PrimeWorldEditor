#ifndef CPROPERTYNAMEGENERATOR_H
#define CPROPERTYNAMEGENERATOR_H

#include "Core/IProgressNotifier.h"
#include <Common/Common.h>

/** Parameters for using the name generator */
struct SPropertyNameGenerationParameters
{
    /** Maximum number of words per name; name generation will complete when all possibilities have been checked */
    int MaxWords;

    /** Prefix to include at the beginning of every name */
    TString Prefix;

    /** Suffix to include at the end of every name */
    TString Suffix;

    /** List of valid type suffixes */
    std::vector<TString> TypeNames;

    /** Whether to separate words with underscores */
    bool UseUnderscores;

    /** Whether to print the output from the generation process to the log */
    bool PrintToLog;
};

/** A generated property name */
struct SGeneratedPropertyName
{
    TString Name;
    TString Type;
    u32 ID;
    std::set<TString> XmlList;
};

/** Generates property names and validates them against know property IDs. */
class CPropertyNameGenerator
{
    /** Whether we have started loading the word list */
    bool mWordListLoadStarted;

    /** Whether the word list has been fully loaded */
    bool mWordListLoadFinished;

    /** Whether the generation process is running */
    bool mIsRunning;

    /** Whether the generation process finished running */
    bool mFinishedRunning;

    /** List of words */
    struct SWord
    {
        TString Word;
        int Usages;
    };
    std::vector<SWord> mWords;

    /** List of output generated property names */
    std::list<SGeneratedPropertyName> mGeneratedNames;

    /** List of word indices */
    std::vector<int> mWordIndices;

public:
    /** Default constructor */
    CPropertyNameGenerator();

    /** Prepares the generator for running name generation */
    void Warmup();

    /** Run the name generation system */
    void Generate(const SPropertyNameGenerationParameters& rkParams, IProgressNotifier* pProgressNotifier);

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
