#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#include "utils.h"
#include <QSettings>
#include <QFileInfo>

class DFInstance;

class MemoryLayout {
public:
    explicit MemoryLayout(DFInstance *df, const QFileInfo &fileinfo);
    MemoryLayout(DFInstance *df, const QFileInfo &fileinfo, const QSettings &data);
    ~MemoryLayout();

    typedef enum{
        MEM_UNK = -1,
        MEM_GLOBALS,
        MEM_LANGUAGE,
        MEM_UNIT,
        MEM_SQUAD,
        MEM_WORD,
        MEM_RACE,
        MEM_CASTE,
        MEM_HIST_FIG,
        MEM_HIST_EVT,
        MEM_HIST_ENT,
        MEM_WEP_SUB,
        MEM_MAT,
        MEM_PLANT,
        MEM_ITEM_SUB,
        MEM_DESC,
        MEM_HEALTH,
        MEM_WOUND,
        MEM_ITEM,
        MEM_ITEM_FILTER,
        MEM_ARMOR_SUB,
        MEM_GEN_REF,
        MEM_SYN,
        MEM_EMOTION,
        MEM_ACTIVITY,
        MEM_JOB,
        MEM_SOUL,
        MEM_COUNT
    } MEM_SECTION;

    typedef enum{
        INVALID_FLAGS_1,
        INVALID_FLAGS_2,
        INVALID_FLAGS_3,
        FLAG_TYPE_COUNT
    } UNIT_FLAG_TYPE;

    static const QString section_name(const MEM_SECTION &section){
        QMap<MEM_SECTION,QString> m;
        m[MEM_UNK] = "UNK";
        m[MEM_GLOBALS] = "addresses";
        m[MEM_LANGUAGE] = "offsets";
        m[MEM_UNIT] = "dwarf_offsets";
        m[MEM_SQUAD] = "squad_offsets";
        m[MEM_WORD] = "word_offsets";
        m[MEM_RACE] = "race_offsets";
        m[MEM_CASTE] = "caste_offsets";
        m[MEM_HIST_FIG] = "hist_figure_offsets";
        m[MEM_HIST_EVT] = "hist_event_offsets";
        m[MEM_HIST_ENT] = "hist_entity_offsets";
        m[MEM_WEP_SUB] = "weapon_subtype_offsets";
        m[MEM_MAT] = "material_offsets";
        m[MEM_PLANT] = "plant_offsets";
        m[MEM_ITEM_SUB] = "item_subtype_offsets";
        m[MEM_DESC] = "descriptor_offsets";
        m[MEM_HEALTH] = "health_offsets";
        m[MEM_WOUND] = "unit_wound_offsets";
        m[MEM_ITEM] = "item_offsets";
        m[MEM_ITEM_FILTER] = "item_filter_offsets";
        m[MEM_ARMOR_SUB] = "armor_subtype_offsets";
        m[MEM_GEN_REF] = "general_ref_offsets";
        m[MEM_SYN] = "syndrome_offsets";
        m[MEM_EMOTION] = "emotion_offsets";
        m[MEM_ACTIVITY] = "activity_offsets";
        m[MEM_JOB] = "job_details";
        m[MEM_SOUL] = "soul_details";
        return m.value(section,m.value(MEM_UNK));
    }

    static const QString flag_type_name(const UNIT_FLAG_TYPE &flag_type){
        QMap<UNIT_FLAG_TYPE,QString> m;
        m[INVALID_FLAGS_1] = "invalid_flags_1";
        m[INVALID_FLAGS_2] = "invalid_flags_2";
        m[INVALID_FLAGS_3] = "invalid_flags_3";
        return m.value(flag_type);
    }

    QSettings &data() { return m_data; }
    QString filename() {return m_fileinfo.fileName();}
    QString filepath() {return m_fileinfo.absoluteFilePath();}
    bool is_valid();
    bool is_complete() {return m_complete;}
    QString game_version() {return m_game_version;}
    QString checksum() {return m_checksum;}
    QString git_sha() {return m_git_sha;}
    uint string_buffer_offset();
    uint string_length_offset();
    uint string_cap_offset();

    QHash<QString, VPTRDIFF> get_section_offsets(const MEM_SECTION &section) {
        return m_offsets.value(section);
    }
    VPTRDIFF offset(const MEM_SECTION &section, const QString &name) const{
        return m_offsets.value(section).value(name,-1);
    }
    QHash<uint,QString> get_flags(const UNIT_FLAG_TYPE &flag_type){
        return m_flags.value(flag_type);
    }

    VPTR address(const QString &key);

    VPTRDIFF global_offset(const QString &key) const {return offset(MEM_GLOBALS,key);}
    VPTRDIFF language_offset(const QString &key) const {return offset(MEM_LANGUAGE,key);}
    VPTRDIFF dwarf_offset(const QString &key) const {return offset(MEM_UNIT,key);}
    VPTRDIFF squad_offset(const QString & key) const {return offset(MEM_SQUAD,key);}
    VPTRDIFF word_offset(const QString & key) const {return offset(MEM_WORD,key);}
    VPTRDIFF race_offset(const QString & key) const {return offset(MEM_RACE,key);}
    VPTRDIFF caste_offset(const QString & key) const {return offset(MEM_CASTE,key);}
    VPTRDIFF hist_figure_offset(const QString & key) const {return offset(MEM_HIST_FIG,key);}
    VPTRDIFF hist_event_offset(const QString & key) const {return offset(MEM_HIST_EVT,key);}
    VPTRDIFF hist_entity_offset(const QString & key) const {return offset(MEM_HIST_ENT,key);}
    VPTRDIFF weapon_subtype_offset(const QString & key) const {return offset(MEM_WEP_SUB,key);}
    VPTRDIFF material_offset(const QString & key) const {return offset(MEM_MAT,key);}
    VPTRDIFF plant_offset(const QString & key) const {return offset(MEM_PLANT,key);}
    VPTRDIFF item_subtype_offset(const QString & key) const {return offset(MEM_ITEM_SUB,key);}
    VPTRDIFF descriptor_offset(const QString & key) const {return offset(MEM_DESC,key);}
    VPTRDIFF health_offset(const QString & key) const {return offset(MEM_HEALTH,key);}
    VPTRDIFF wound_offset(const QString & key) const {return offset(MEM_WOUND,key);}
    VPTRDIFF item_offset(const QString & key) const {return offset(MEM_ITEM,key);}
    VPTRDIFF item_filter_offset(const QString & key) const {return offset(MEM_ITEM_FILTER,key);}
    VPTRDIFF armor_subtype_offset(const QString & key) const {return offset(MEM_ARMOR_SUB,key);}
    VPTRDIFF general_ref_offset(const QString & key) const {return offset(MEM_GEN_REF,key);}
    VPTRDIFF syndrome_offset(const QString & key) const {return offset(MEM_SYN,key);}
    VPTRDIFF emotion_offset(const QString & key) const {return offset(MEM_EMOTION,key);}
    VPTRDIFF activity_offset(const QString & key) const {return offset(MEM_ACTIVITY,key);}
    VPTRDIFF job_detail(const QString &key) const {return offset(MEM_JOB,key);}
    VPTRDIFF soul_detail(const QString &key) const {return offset(MEM_SOUL,key);}

    QHash<uint, QString> invalid_flags_1() {return get_flags(INVALID_FLAGS_1) ;}
    QHash<uint, QString> invalid_flags_2() {return get_flags(INVALID_FLAGS_2);}
    QHash<uint, QString> invalid_flags_3() {return get_flags(INVALID_FLAGS_3);}

    //Setters
    void set_address(const QString & key, uint value);
    void set_game_version(const QString & value);
    void set_checksum(const QString & checksum);
    void save_data();
    void set_complete();

    bool operator<(const MemoryLayout & rhs) const {
        return m_game_version < rhs.m_game_version;
    }

    void load_data();

private:
    DFInstance *m_df;

    QHash<MEM_SECTION, QHash<QString, VPTRDIFF> > m_offsets;
    QHash<UNIT_FLAG_TYPE, QHash<uint,QString> > m_flags;

    QFileInfo m_fileinfo;
    QString m_checksum;
    QString m_git_sha;
    QString m_game_version;
    QSettings m_data;
    bool m_complete;

    uint read_hex(QString key);
    void read_group(const MEM_SECTION &section);
    void read_flags(const UNIT_FLAG_TYPE &flag_type);
};
Q_DECLARE_METATYPE(MemoryLayout *)
#endif
