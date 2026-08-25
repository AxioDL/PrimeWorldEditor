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
        return fmt::format("{}-{}-{}-{}-{}", 
            fmt::join(std::vector<char>{Value(pData)[3], Value(pData)[2], Value(pData)[1], Value(pData)[0]} | std::views::transform(to_hex), ""),
            fmt::join(std::vector<char>{Value(pData)[5], Value(pData)[4]} | std::views::transform(to_hex), ""),
            fmt::join(std::vector<char>{Value(pData)[7], Value(pData)[6]} | std::views::transform(to_hex), ""),
            fmt::join(std::vector<char>{Value(pData)[8], Value(pData)[9]} | std::views::transform(to_hex), ""),
            fmt::join(std::vector<char>{Value(pData)[10], Value(pData)[11], Value(pData)[12], Value(pData)[13], Value(pData)[14], Value(pData)[15]} | std::views::transform(to_hex), "")
        );
    };
};

#endif // CGUIDPROPERTY_H
