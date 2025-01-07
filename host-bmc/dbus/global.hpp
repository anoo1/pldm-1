#pragma once

#include "serialize.hpp"

#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/server/object.hpp>
#include <xyz/openbmc_project/Inventory/Item/Global/server.hpp>

#include <string>

namespace pldm
{
namespace dbus
{
using ItemGlobal = sdbusplus::server::object_t<
    sdbusplus::xyz::openbmc_project::Inventory::Item::server::Global>;

class Global : public ItemGlobal
{
  public:
    Global() = delete;
    ~Global() = default;
    Global(const Global&) = delete;
    Global& operator=(const Global&) = delete;
    Global(Global&&) = delete;
    Global& operator=(Global&&) = delete;

    Global(sdbusplus::bus_t& bus, const std::string& objPath) :
        ItemGlobal(bus, objPath.c_str())
    {
        pldm::serialize::Serialize::getSerialize().serialize(objPath, "Global");
    }
};

} // namespace dbus
} // namespace pldm
