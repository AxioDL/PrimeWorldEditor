#include "Core/NCoreTests.h"

#include "Core/IUIRelay.h"
#include "Core/GameProject/CGameProject.h"
#include "Core/GameProject/CResourceEntry.h"
#include "Core/Resource/EResType.h"
#include "Core/Resource/Cooker/CResourceCooker.h"
#include <Common/FileUtil.h>
#include <Common/Log.h>
#include <Common/FileIO/CFileInStream.h>
#include <Common/FileIO/CFileOutStream.h>
#include <Common/Math/MathUtil.h>

#include <algorithm>
#include <span>
#include <string_view>

#include <fmt/format.h>

namespace NCoreTests
{
namespace
{
/** Checks for a parameter in the commandline stream */
static const char* ParseParameter(std::string_view parmName, int argc, char* argv[])
{
    // Valid length needs to at minimum be greater than option and an = (e.g. -option=test)
    // otherwise there's nothing on the right side of the '=' that we can find.
    const auto args = std::span<char*>(argv, size_t(argc));
    const auto minValidLength = parmName.size() + 2;

    // Find our parameter
    const auto strIter = std::ranges::find_if(args, [&parmName](std::string_view str) { return str.contains(parmName); });
    if (strIter == args.end())
        return nullptr;

    // See if we can extract the value
    const auto str = std::string_view(*strIter);
    if (str.size() < minValidLength)
        return nullptr;

    const auto valueIdx = str.find('=');
    if (valueIdx == std::string_view::npos)
        return nullptr;

    return &str[valueIdx + 1];
}

/** Checks for the existence of a token in the commandline stream */
static bool ParseToken(std::string_view token, int argc, char* argv[])
{
    return std::ranges::contains(argv, argv + argc, token);
}

static std::string_view ValidateUsageString()
{
    return "Usage: ValidateCooker -type=<ResourceType> [-allowdump] [-project=<Project>]";
}

struct CompareDataResult
{
    bool valid{};
    std::string error;

    explicit operator bool() const { return valid; }
};

static CompareDataResult CompareData(std::span<const uint8_t> lhs, std::span<const char> rhs, size_t compareSize)
{
    const auto lhsEnd = lhs.begin() + compareSize;
    const auto [first1, first2] = std::mismatch(lhs.begin(), lhsEnd, rhs.begin(),
                                                [](auto l, auto r) { return l == uint8_t(r); });

    // No mismatch
    if (first1 == lhsEnd)
        return {true, ""};

    const auto mismatchIdx = std::distance(lhs.begin(), first1);
    return {false, fmt::format("Data mismatch at index {} (0x{:X}):\nOriginal data: 0x{:X}, Cooked data: 0x{:X}\n\n", mismatchIdx, mismatchIdx, *first1, *first2)};
}

/** Validate all cooker output for the given resource type matches the original asset data */
static bool ValidateCooker(EResourceType ResourceType, bool DumpInvalidFileContents, const CGameProject* project)
{
    NLog::Debug("Validating output of {} cooker...",
                TEnumReflection<EResourceType>::ConvertValueToString(ResourceType));

    const TString ResourcesDir = project->ResourcesDir(false);
    uint32_t NumValid = 0, NumInvalid = 0;

    // Iterate through all resources
    for (const auto& resource : project->ResourceStore()->MakeTypedResourceView(ResourceType))
    {
        if (!resource->HasCookedVersion())
            continue;

        // Get original cooked data
        const TString CookedPath = resource->CookedAssetPath(true);
        CFileInStream FileStream(ResourcesDir / CookedPath, std::endian::big);

        if (!FileStream.IsValid())
            continue;

        std::vector<uint8_t> OriginalData(FileStream.Size());
        FileStream.ReadBytes(OriginalData.data(), OriginalData.size());
        FileStream.Close();

        // Generate new cooked data
        std::vector<char> NewData;
        CVectorOutStream MemoryStream(&NewData, std::endian::big);
        CResourceCooker::CookResource(resource.get(), MemoryStream);

        // Start our comparison by making sure the sizes match up
        const size_t kAlignment         = (resource->Game() >= EGame::Corruption ? 64 : 32);
        const auto kAlignedOriginalSize = Math::Align(OriginalData.size(), kAlignment);
        const auto kAlignedNewSize      = Math::Align(NewData.size(), kAlignment);
        std::string InvalidReason;
        bool IsValid = false;

        if (kAlignedOriginalSize == kAlignedNewSize &&
            OriginalData.size() >= NewData.size())
        {
            // Compare actual data. Note that the original asset can have alignment padding
            // at the end, which is applied by the pak but usually preserved in extracted
            // files. We do not include this in the comparison as missing padding does not
            // indicate malformed data.
            const auto DataSize = std::min(OriginalData.size(), NewData.size());

            if (auto compareResult = CompareData(OriginalData, NewData, DataSize))
            {
                // Verify any missing data at the end is padding.
                bool MissingData = false;

                if (OriginalData.size() > NewData.size())
                {
                    MissingData = std::any_of(OriginalData.begin() + DataSize, OriginalData.end(),
                                              [](auto value) { return value != 0xFF; });
                }

                if (!MissingData)
                {
                    // All tests passed!
                    IsValid = true;
                }
                else
                {
                    InvalidReason = "missing data";
                }
            }
            else
            {
                InvalidReason = std::move(compareResult.error);
            }
        }
        else
        {
            InvalidReason = "size mismatch";
        }

        // Print test results
        if (IsValid)
        {
            NLog::Debug("[SUCCESS] {}", CookedPath);
            NumValid++;
        }
        else
        {
            NLog::Debug("[FAILED: {}] {}", InvalidReason, CookedPath);
            NumInvalid++;
        }

        if (DumpInvalidFileContents)
        {
            const TString DumpPath = "dump" / CookedPath;
            FileUtil::MakeDirectory(DumpPath.GetFileDirectory());

            CFileOutStream DumpFile(DumpPath, std::endian::big);
            DumpFile.WriteBytes(NewData.data(), NewData.size());
            DumpFile.Close();
        }

        if (NumInvalid >= 100)
        {
            NLog::Debug("Test aborted; at least 100 invalid resources. Checked {} resources, {} passed, {} failed",
                        NumValid + NumInvalid, NumValid, NumInvalid);
            return false;
        }
    }

    // Test complete
    const bool TestSuccess = (NumInvalid == 0);
    NLog::Debug("Test {}; checked {} resources, {} passed, {} failed",
                TestSuccess ? "SUCCEEDED" : "FAILED",
                NumValid + NumInvalid, NumValid, NumInvalid);

    return TestSuccess;
}
} // Anonymous namespace

/** Check commandline input to see if the user is running a test */
bool RunTests(int argc, char* argv[])
{
    if (ParseToken("ValidateCooker", argc, argv))
    {
        // Fetch parameters
        const char* pkType = ParseParameter("-type", argc, argv);
        if (!pkType)
        {
            GetUIRelay()->ShowMessageBox("Error", fmt::format("Missing or invalid -type option\n\n{}",
                                         ValidateUsageString()));
            return true;
        }

        const auto Type = TEnumReflection<EResourceType>::ConvertStringToValue(pkType);
        const bool AllowDump = ParseToken("-allowdump", argc, argv);
        if (Type == EResourceType::Invalid)
        {
            GetUIRelay()->ShowMessageBox("ValidateCooker", fmt::format("Invalid -type value: Non existing resource type\n\n{}",
                                         ValidateUsageString()));
            return true;
        }

        const char* projectPath = ParseParameter("-project", argc, argv);
        if (!projectPath)
        {
            // Indicates that we'd like to manually select the project file.
            projectPath = "";
        }

        const auto [success, project] = GetUIRelay()->OpenProject(projectPath);
        if (!success)
        {
            GetUIRelay()->ShowMessageBox("Error", fmt::format("Failed to open project at path: {}", projectPath));
            return true;
        }

        ValidateCooker(Type, AllowDump, project);
        return true;
    }

    // No test being run.
    return false;
}

} // end namespace NCoreTests
