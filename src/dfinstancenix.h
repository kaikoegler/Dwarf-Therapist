#ifndef DFINSTANCENIX_H
#define DFINSTANCENIX_H

#include "dfinstance.h"
#include "utils.h"

class DFInstanceNix : public DFInstance
{
    Q_OBJECT
public:
    DFInstanceNix(QObject *parent);

    QString read_string(const VPTR addr);
    size_t write_string(const VPTR addr, const QString &str);

    bool df_running();

protected:
    pid_t m_pid;
    QString calculate_checksum();

    VPTR get_string(const QString &str);
    virtual VPTR alloc_chunk(size_t size) = 0;

    QString m_loc_of_dfexe;

private:
    QHash<QString, VPTR> m_string_cache;
};

#endif // DFINSTANCENIX_H
