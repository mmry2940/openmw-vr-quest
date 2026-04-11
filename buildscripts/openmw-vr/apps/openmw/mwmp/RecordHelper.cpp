#include <components/openmw-mp/TimedLog.hpp>

#include "RecordHelper.hpp"

namespace
{
    template <typename T>
    void ignoreOverride(const T&)
    {
        LOG_MESSAGE_SIMPLE(TimedLog::LOG_VERBOSE, "Ignoring record override in compatibility mode");
    }
}

void RecordHelper::overrideRecord(const mwmp::ActivatorRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::ApparatusRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::ArmorRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::BodyPartRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::BookRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::CellRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::ClothingRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::ContainerRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::CreatureRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::DoorRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::EnchantmentRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::GameSettingRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::IngredientRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::LightRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::LockpickRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::MiscellaneousRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::NpcRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::PotionRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::ProbeRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::RepairRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::ScriptRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::SoundRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::SpellRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::StaticRecord& record) { ignoreOverride(record); }
void RecordHelper::overrideRecord(const mwmp::WeaponRecord& record) { ignoreOverride(record); }

void RecordHelper::createPlaceholderInteriorCell()
{
    // Compatibility no-op.
}

const std::string RecordHelper::getPlaceholderInteriorCellName()
{
    return placeholderInteriorCellName;
}
