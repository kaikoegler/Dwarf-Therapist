#ifndef BODYPART_H
#define BODYPART_H

#include "flagarray.h"
#include "bodypartlayer.h"

class DFInstance;
class Race;

class BodyPart
{
public:

    BodyPart();

    BodyPart(DFInstance *df, Race *r, VPTR bp_addr, int bp_id);

    virtual ~BodyPart();

    BodyPartLayer get_layer(short id);

    QHash<int, BodyPartLayer> get_layers();

    QString name() {return m_bp_name;}
    int id() {return m_body_part_id;}
    QString token() {return m_token;}
    int parent() {return m_parent_id;}
    FlagArray flags() {return m_flags;}

private:
    DFInstance *m_df;
    Race *m_race;
    VPTR bp_addr;
    int m_body_part_id;
    int m_parent_id;

    QString m_bp_name;
    QString m_token;
    QHash<int, BodyPartLayer> m_layers;
    QVector<VPTR> m_layers_addr;
    FlagArray m_flags;

    void build_bp_name();

};
#endif // BODYPART_H
