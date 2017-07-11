/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef DFINSTANCE_H
#define DFINSTANCE_H

#include "utils.h"
#include "global_enums.h"
#include "truncatingfilelogger.h"

#include <QDir>
#include <QPointer>

#ifdef Q_OS_WIN
typedef int PID;
#else
#include <unistd.h>
typedef pid_t PID;
#endif

class Dwarf;
class FortressEntity;
class ItemSubtype;
class ItemWeaponSubtype;
class Languages;
class Material;
class MemoryLayout;
class Plant;
class Race;
class Reaction;
class Squad;
class Word;
class EmotionGroup;
class Activity;
class EquipWarn;

class DFInstance : public QObject {
    Q_OBJECT
public:

    DFInstance(QObject *parent=0);
    virtual ~DFInstance();

    virtual void find_running_copy() = 0;
    virtual bool df_running() = 0;

    static quint32 ticks_per_day;
    static quint32 ticks_per_month;
    static quint32 ticks_per_season;
    static quint32 ticks_per_year;

    typedef enum{
        DFS_DISCONNECTED = -1,
        DFS_CONNECTED,
        DFS_LAYOUT_OK,
        DFS_GAME_LOADED
    } DFI_STATUS;

    // accessors
    VPTR df_base_addr() const {return m_base_addr;}
    const QString df_checksum() {return m_df_checksum;}
    const QString layout_subdir();
    DFI_STATUS status() const {return m_status;}
    short dwarf_race_id() {return m_dwarf_race_id;}
    QList<MemoryLayout*> get_layouts() { return m_memory_layouts.values(); }
    QDir get_df_dir() { return m_df_dir; }
    quint32 current_year() {return m_current_year;}
    int dwarf_civ_id() {return m_dwarf_civ_id;}
    const QStringList status_err_msg();

    // memory reading
    template<typename T> T read_mem(VPTR addr, bool no_failsafe = false) {
        T buf;
        size_t rv = read_raw(addr, sizeof(T), &buf);
        if (!no_failsafe && rv != sizeof(T))
            buf = T();
        return buf;
    }
    virtual size_t read_raw(VPTR addr, size_t bytes, void *buf) = 0;
    virtual QString read_string(VPTR addr) = 0;
    size_t read_raw(VPTR addr, size_t bytes, QByteArray &buffer);
    quint8 read_byte(VPTR addr);
    quint32 read_word(VPTR addr);
    VPTR read_addr(VPTR addr);
    qint16 read_short(VPTR addr);
    qint32 read_int(VPTR addr);
    QVector<VPTR> enumerate_vector(VPTR addr);
    QVector<qint16> enumerate_vector_short(VPTR addr);
    template<typename T>
    QVector<T> enum_vec(VPTR addr) {
        QVector<T> out;
        VPTR start = read_addr(addr);
        VPTR end = read_addr(addr + sizeof(VPTR ));
        size_t bytes = end - start;
        if (bytes % sizeof(T)) {
            LOGE << "VECTOR SIZE IS NOT A MULTIPLE OF TYPE";
        } else {
            out.resize(bytes / sizeof(T));
            size_t bytes_read = read_raw(start, bytes, out.data());
            TRACE << "Found" << bytes_read / sizeof(T) << "things in vector at" << hexify(addr);
        }
        return out;
    }
    Word * read_dwarf_word(VPTR addr);
    QString read_dwarf_name(VPTR addr);

    QString pprint(const QByteArray &ba);

    // Memory layouts
    MemoryLayout *memory_layout() {return m_layout;}
    void set_memory_layout(QString checksum = QString());
    MemoryLayout *get_memory_layout(QString checksum);
    MemoryLayout *find_memory_layout(QString git_sha);
    bool add_new_layout(const QString & filename, const QString data, QString &result_msg);

    // Writing
    virtual size_t write_raw(VPTR addr, size_t bytes, const void *buffer) = 0;
    size_t write_raw(VPTR addr, size_t bytes, const QByteArray &buffer);
    virtual size_t write_string(VPTR addr, const QString &str) = 0;
    size_t write_int(VPTR addr, int val);

    bool is_attached() {return m_attach_count > 0;}
    virtual bool attach() = 0;
    virtual bool detach() = 0;
    virtual int VM_TYPE_OFFSET() {return 0x1;}

    static bool authorize();
    quint32 current_year_time() {return m_cur_year_tick;}
    quint32 current_time() {return m_cur_time;}
    static DFInstance * newInstance();

    // Methods for when we know how the data is layed out
    void load_game_data();
    void read_raws();

    QVector<Dwarf*> load_dwarves();
    void load_reactions();
    void load_races_castes();
    void load_main_vectors();

    void load_item_defs();
    void load_items();

    void load_fortress();
    void load_fortress_name();

    void load_activities();

    void refresh_data();

    QList<Squad*> load_squads();
    Squad * get_squad(int id);

    int get_labor_count(int id) const {return m_enabled_labor_count.value(id,0);}
    void update_labor_count(int id, int change)
    {
        m_enabled_labor_count[id] += change;
    }

    QString get_language_word(VPTR addr);
    QString get_translated_word(VPTR addr);
    QString get_name(VPTR addr, bool translate);

    Reaction * get_reaction(QString tag) { return m_reactions.value(tag, 0); }
    Race * get_race(const uint & offset) { return m_races.value(offset, NULL); }
    QVector<Race *> get_races() {return m_races;}

    VPTR find_historical_figure(int hist_id);
    VPTR find_identity(int id);
    VPTR find_event(int id);
    QPair<int, QString> find_activity(int histfig_id);
    VPTR find_occupation(int histfig_id);

    FortressEntity * fortress() {return m_fortress;}

    struct pref_stat{
        QStringList names_likes;
        QStringList names_dislikes;
        QString pref_category;
    };

    VPTR get_syndrome(int idx) {
        return m_all_syndromes.value(idx);
    }
    VPTR get_material_template(QString temp_id) {return m_material_templates.value(temp_id);}
    QVector<Material *> get_inorganic_materials() {return m_inorganics_vector;}
    QHash<ITEM_TYPE, QVector<VPTR> > get_all_item_defs() {return m_itemdef_vectors;}
    QVector<VPTR>  get_colors() {return m_color_vector;}
    QVector<VPTR> get_shapes() {return m_shape_vector;}
    QVector<Plant *> get_plants() {return m_plants_vector;}
    QVector<Material *> get_base_materials() {return m_base_materials;}

    QString get_building_name(int id);

    ItemWeaponSubtype* find_weapon_subtype(QString name);
    QMap<QString, ItemWeaponSubtype *> get_ordered_weapon_defs() {return m_ordered_weapon_defs;}

    QList<ItemSubtype*> get_item_subtypes(ITEM_TYPE itype){return m_item_subtypes.value(itype);}
    ItemSubtype* get_item_subtype(ITEM_TYPE itype, int sub_type);

    Material * find_material(int mat_index, short mat_type);

    QVector<VPTR> get_itemdef_vector(ITEM_TYPE i);
    VPTR get_item_address(ITEM_TYPE itype, int item_id);

    QString get_preference_item_name(int index, int subtype);
    QString get_preference_other_name(int index, PREF_TYPES p_type);
    QString get_artifact_name(ITEM_TYPE itype,int item_id);

    inline Material * get_inorganic_material(int index) {
        return m_inorganics_vector.value(index);
    }
    inline Material * get_raw_material(int index) {
        return m_base_materials.value(index);
    }
    inline Plant * get_plant(int index) {
        return m_plants_vector.value(index);
    }
    QString find_material_name(int mat_index, short mat_type, ITEM_TYPE itype, MATERIAL_STATES mat_state = SOLID);
    const QHash<QPair<QString,QString>,pref_stat*> get_preference_stats() {return m_pref_counts;}
    const QHash<int, EmotionGroup*> get_emotion_stats() {return m_emotion_counts;}
    const QHash<ITEM_TYPE,EquipWarn*> get_equip_warnings(){return m_equip_warning_counts;}

    const QString fortress_name();
    QList<Squad*> squads() {return m_squads;}

protected:
    VPTR m_base_addr;
    QString m_df_checksum;
    MemoryLayout *m_layout;
    int m_attach_count;
    QTimer *m_heartbeat_timer;
    short m_dwarf_race_id;
    int m_dwarf_civ_id;
    quint32 m_current_year;
    QDir m_df_dir;
    QVector<Dwarf*> m_actual_dwarves;
    QVector<Dwarf*> m_labor_capable_dwarves;
    quint32 m_cur_year_tick;
    quint32 m_cur_time;
    QHash<int,int> m_enabled_labor_count;
    DFI_STATUS m_status;

    virtual bool set_pid() = 0;

    void load_population_data();
    void load_role_ratings();
    bool check_vector(VPTR start, VPTR end, VPTR addr);

    /*! this hash will hold a map of all loaded and valid memory layouts found
        on disk, the key is a QString of the checksum since other OSs will use
        an MD5 of the binary instead of a PE timestamp */
    QHash<QString, MemoryLayout*> m_memory_layouts; // checksum->layout

    static PID select_pid(QSet<PID> pids);

    VPTR alloc_chunk(size_t size);
    virtual bool mmap(size_t size) = 0;
    virtual bool mremap(size_t new_size) = 0;
    VPTR m_alloc_start;
    size_t m_alloc_len;
    size_t m_alloc_capacity;

private slots:
    void heartbeat();

signals:
    // methods for sending progress information to QWidgets
    void connection_interrupted();
    void progress_message(const QString &message);
    void progress_range(int min, int max);
    void progress_value(int value);

private:
    Languages* m_languages;
    FortressEntity* m_fortress;
    QHash<QString, Reaction *> m_reactions;
    QVector<Race *> m_races;

    QMap<QString, ItemWeaponSubtype *> m_ordered_weapon_defs;
    QHash<ITEM_TYPE,QList<ItemSubtype *> > m_item_subtypes;
    QVector<Plant *> m_plants_vector;
    QVector<Material *> m_inorganics_vector;
    QVector<Material *> m_base_materials;

    QVector<VPTR> get_creatures(bool report_progress = true);

    QHash<int,VPTR> m_hist_figures;
    QVector<VPTR> m_fake_identities;
    QHash<int,VPTR> m_occupations;
    QHash<int,VPTR> m_events;
    QMap<int,QPointer<Activity> > m_activities;

    QHash<ITEM_TYPE, QVector<VPTR> > m_itemdef_vectors;
    QHash<ITEM_TYPE, QVector<VPTR> > m_items_vectors;
    QHash<ITEM_TYPE, QHash<int,VPTR> >  m_mapped_items;

    QVector<VPTR> m_color_vector;
    QVector<VPTR> m_shape_vector;
    QVector<VPTR> m_poetic_vector;
    QVector<VPTR> m_music_vector;
    QVector<VPTR> m_dance_vector;

    QHash<QString, VPTR> m_material_templates;

    QVector<VPTR> m_all_syndromes;

    QHash<ITEM_TYPE,EquipWarn*> m_equip_warning_counts;
    QHash<QPair<QString,QString>, pref_stat*> m_pref_counts;
    QHash<int, EmotionGroup*> m_emotion_counts;

    QString m_fortress_name;
    QString m_fortress_name_translated;

    VPTR m_squad_vector;
    QList<Squad*> m_squads;

    void load_hist_figures();
    void load_occupations();
    void index_item_vector(ITEM_TYPE itype);
    void send_connection_interrupted();
};

#endif // DFINSTANCE_H
