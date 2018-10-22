#ifndef CTEMPLATELISTVIEW
#define CTEMPLATELISTVIEW

#include "CTemplateMimeData.h"
#include "Editor/UICommon.h"
#include <Core/Resource/Script/CGameTemplate.h>
#include <QAbstractListModel>
#include <QDrag>
#include <QListView>

class CTemplateListModel : public QAbstractListModel
{
    Q_OBJECT
    CGameTemplate *mpGame;
    QList<CScriptTemplate*> mTemplates;

public:
    CTemplateListModel(QObject *pParent = 0)
        : QAbstractListModel(pParent)
    {}

    int rowCount(const QModelIndex&) const
    {
        return mTemplates.size();
    }

    QVariant data(const QModelIndex& rkIndex, int Role) const
    {
        if (Role == Qt::DisplayRole || Role == Qt::ToolTipRole)
            return TO_QSTRING(mTemplates[rkIndex.row()]->Name());
        else
            return QVariant::Invalid;
    }

    Qt::DropActions supportedDropActions() const
    {
        return Qt::IgnoreAction;
    }

    Qt::DropActions supportedDragActions() const
    {
        return Qt::MoveAction;
    }

    QMimeData* mimeData(const QModelIndexList& rkIndices) const
    {
        if (rkIndices.size() != 1) return nullptr;
        QModelIndex Index = rkIndices.front();
        CScriptTemplate *pTemp = TemplateForIndex(Index);
        return new CTemplateMimeData(pTemp);
    }

    Qt::ItemFlags flags(const QModelIndex&) const
    {
        return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
    }

    void SetGame(CGameTemplate *pGame)
    {
        beginResetModel();

        mpGame = pGame;
        mTemplates.clear();

        if (mpGame)
        {
            for (u32 iTemp = 0; iTemp < mpGame->NumScriptTemplates(); iTemp++)
                mTemplates << mpGame->TemplateByIndex(iTemp);

            qSort(mTemplates.begin(), mTemplates.end(), [](CScriptTemplate *pLeft, CScriptTemplate *pRight) -> bool {
                return pLeft->Name() < pRight->Name();
            });
        }

        endResetModel();
    }

    inline CScriptTemplate* TemplateForIndex(const QModelIndex& rkIndex) const
    {
        return mTemplates[rkIndex.row()];
    }
};

class CTemplateListView : public QListView
{
    Q_OBJECT
    CTemplateListModel *mpModel;

public:
    CTemplateListView(QWidget *pParent = 0)
        : QListView(pParent)
    {
        setModel(new CTemplateListModel(this));
    }

    void setModel(QAbstractItemModel *pModel)
    {
        if (CTemplateListModel *pTempModel = qobject_cast<CTemplateListModel*>(pModel))
            mpModel = pTempModel;
        else
            mpModel = nullptr;

        QListView::setModel(mpModel);
    }

    inline void SetGame(CGameTemplate *pGame)
    {
        if (mpModel) mpModel->SetGame(pGame);
    }

protected:
    void startDrag(Qt::DropActions)
    {
        QModelIndexList Selection = selectionModel()->selectedRows();

        if (Selection.size() == 1 && mpModel)
        {
            QDrag *pDrag = new QDrag(this);
            pDrag->setMimeData(mpModel->mimeData(Selection));
            pDrag->setPixmap(QPixmap());
            pDrag->exec();
        }
    }
};

#endif // CTEMPLATELISTVIEW

