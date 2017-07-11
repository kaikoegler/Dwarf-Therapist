#include "dfinstance.h"
#include "itemsubtype.h"
#include "memorylayout.h"

ItemSubtype::ItemSubtype(ITEM_TYPE itype, DFInstance *df, VPTR address, QObject *parent)
    : QObject(parent)
    , m_address(address)
    , m_df(df)
    , m_mem(df->memory_layout())
    , m_iType(itype)
    , m_subType(-1)
{
    set_base_offsets();
}

ItemSubtype::ItemSubtype(DFInstance *df, VPTR address, QObject *parent)
    : QObject(parent)
    , m_address(address)
    , m_df(df)
    , m_mem(df->memory_layout())
    , m_iType(NONE)
    , m_subType(-1)
{
    set_base_offsets();
}

void ItemSubtype::set_base_offsets()
{
    m_offset_adj = m_mem->item_subtype_offset("adjective");
    m_offset_mat = -1;
    m_offset_preplural = -1;
}

void ItemSubtype::read_data() {
    if(m_address){
        m_subType = m_df->read_short(m_address + m_mem->item_subtype_offset("sub_type"));

        QString mat_name;

        if(m_offset_mat != -1)
            mat_name = m_df->read_string(m_address + m_offset_mat);

        QStringList name_parts;
        if(m_offset_adj != -1)
            name_parts.append(m_df->read_string(m_address + m_offset_adj));
        name_parts.append(mat_name);
        name_parts.append(m_df->read_string(m_address + m_mem->item_subtype_offset("name")));
        m_name = capitalizeEach(name_parts.join(" ")).simplified().trimmed();

        name_parts.removeLast();
        name_parts.append(m_df->read_string(m_address + m_mem->item_subtype_offset("name_plural")));
        m_name_plural = capitalizeEach(name_parts.join(" ")).simplified().trimmed();
    }
}
