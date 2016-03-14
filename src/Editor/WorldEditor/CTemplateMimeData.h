#ifndef CTEMPLATEMIMEDATA_H
#define CTEMPLATEMIMEDATA_H

#include <Core/Resource/Script/CScriptTemplate.h>
#include <QMimeData>

class CTemplateMimeData : public QMimeData
{
    Q_OBJECT
    CScriptTemplate *mpTemplate;

public:
    CTemplateMimeData(CScriptTemplate *pTemplate)
        : mpTemplate(pTemplate)
    {}

    inline CScriptTemplate* Template() const { return mpTemplate; }
};

#endif // CTEMPLATEMIMEDATA_H
