#ifndef BODYPARTLAYER_H
#define BODYPARTLAYER_H

#include "flagarray.h"
#include "global_enums.h"

class DFInstance;
class Race;

class BodyPartLayer{

public:

    BodyPartLayer();

    BodyPartLayer(VPTR layer_addr, int id, DFInstance *df, Race *r);

    int id(){return m_layer_id;}
    QString name(){return m_layer_name;}

    QString tissue_name() {return m_tissue_name;}
    eHealth::TISSUE_TYPE tissue_type(){return m_t_type;}

    int tissue_id(){return m_tissue_id;}
    int global_layer_id() {return m_global_layer_id;}

private:
    VPTR m_addr;
    DFInstance *m_df;
    Race *m_race;
    int m_layer_id;
    QString m_layer_name;
    QString m_tissue_name;

    int m_tissue_id;
    FlagArray m_tissue_flags;
    eHealth::TISSUE_TYPE m_t_type;

    int m_global_layer_id;
};

#endif // BODYPARTLAYER_H
