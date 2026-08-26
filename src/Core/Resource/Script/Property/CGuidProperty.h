#ifndef CGUIDPROPERTY_H
#define CGUIDPROPERTY_H

#include "Core/Resource/Script/Property/IProperty.h"
#include <vector>
#include <ranges>
#include <fmt/format.h>
#include <fmt/ranges.h>

class CGuidProperty : public TTypedProperty<std::vector<char>, EPropertyType::Guid>
{
    friend class IProperty;

protected:
    explicit CGuidProperty(EGame Game)
        : TTypedProperty(Game)
    {}

public:
    void SerializeValue(void* pData, IArchive& Arc) override
    {
        Arc << SerialParameter("Data", ValueRef(pData));
    }

    TString ValueAsString(const void* pData) const override
    {
        // Display as mixed-endian
        constexpr auto to_hex = [](char c)
        {
            return fmt::format("{:02x}", static_cast<unsigned char>(c));
        };

        const auto& Data = ValueRef(pData);
        return fmt::format("{}-{}-{}-{}-{}", 
            fmt::join(std::vector<char>{Data[3],  Data[2], Data[1], Data[0]} | std::views::transform(to_hex), ""),
            fmt::join(std::vector<char>{Data[5],  Data[4]} | std::views::transform(to_hex), ""),
            fmt::join(std::vector<char>{Data[7],  Data[6]} | std::views::transform(to_hex), ""),
            fmt::join(std::vector<char>{Data[8],  Data[9]} | std::views::transform(to_hex), ""),
            fmt::join(std::vector<char>{Data[10], Data[11], Data[12], Data[13], Data[14], Data[15]} | std::views::transform(to_hex), "")
        );
    };
};

#endif // CGUIDPROPERTY_H
