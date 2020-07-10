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
    CGameTemplate *mpGame = nullptr;
    QList<CScriptTemplate*> mTemplates;

public:
    explicit CTemplateListModel(QObject *pParent = nullptr)
        : QAbstractListModel(pParent)
    {}

    int rowCount(const QModelIndex&) const override
    {
        return mTemplates.size();
    }

    QVariant data(const QModelIndex& rkIndex, int Role) const override
    {
        if (Role == Qt::DisplayRole || Role == Qt::ToolTipRole)
            return TO_QSTRING(mTemplates[rkIndex.row()]->Name());
        else
            return QVariant::Invalid;
    }

    Qt::DropActions supportedDropActions() const override
    {
        return Qt::IgnoreAction;
    }

    Qt::DropActions supportedDragActions() const override
    {
        return Qt::MoveAction;
    }

    QMimeData* mimeData(const QModelIndexList& rkIndices) const override
    {
        if (rkIndices.size() != 1)
            return nullptr;

        QModelIndex Index = rkIndices.front();
        CScriptTemplate *pTemp = TemplateForIndex(Index);
        return new CTemplateMimeData(pTemp);
    }

    Qt::ItemFlags flags(const QModelIndex&) const override
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
            for (uint32 iTemp = 0; iTemp < mpGame->NumScriptTemplates(); iTemp++)
                mTemplates.push_back(mpGame->TemplateByIndex(iTemp));

            std::sort(mTemplates.begin(), mTemplates.end(), [](const CScriptTemplate *pLeft, const CScriptTemplate *pRight) {
                return pLeft->Name() < pRight->Name();
            });
        }

        endResetModel();
    }

    CScriptTemplate* TemplateForIndex(const QModelIndex& rkIndex) const
    {
        return mTemplates[rkIndex.row()];
    }
};

class CTemplateListView : public QListView
{
    Q_OBJECT
    CTemplateListModel *mpModel = nullptr;

public:
    explicit CTemplateListView(QWidget *pParent = nullptr)
        : QListView(pParent)
    {
        CTemplateListView::setModel(new CTemplateListModel(this));
    }

    void setModel(QAbstractItemModel* pModel) override
    {
        if (CTemplateListModel *pTempModel = qobject_cast<CTemplateListModel*>(pModel))
            mpModel = pTempModel;
        else
            mpModel = nullptr;

        QListView::setModel(mpModel);
    }

    void SetGame(CGameTemplate *pGame)
    {
        if (mpModel)
            mpModel->SetGame(pGame);
    }

protected:
    void startDrag(Qt::DropActions) override
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

