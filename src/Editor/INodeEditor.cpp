#include "INodeEditor.h"
#include "Editor/Undo/UndoCommands.h"

INodeEditor::INodeEditor(QWidget *pParent)
    : QMainWindow(pParent),
      mShowGizmo(false),
      mGizmoHovering(false),
      mGizmoTransforming(false),
      mTranslateSpace(eWorldTransform),
      mRotateSpace(eWorldTransform)
{
    // Create undo actions
    QAction *pUndoAction = mUndoStack.createUndoAction(this);
    QAction *pRedoAction = mUndoStack.createRedoAction(this);
    pUndoAction->setShortcut(QKeySequence::Undo);
    pRedoAction->setShortcut(QKeySequence::Redo);
    mUndoActions.push_back(pUndoAction);
    mUndoActions.push_back(pRedoAction);

    // Create gizmo actions
    mGizmoActions.append(new QAction(QIcon(":/icons/SelectMode.png"), "Select Objects", this));
    mGizmoActions.append(new QAction(QIcon(":/icons/Translate.png"), "Translate", this));
    mGizmoActions.append(new QAction(QIcon(":/icons/Rotate.png"), "Rotate", this));
    mGizmoActions.append(new QAction(QIcon(":/icons/Scale.png"), "Scale", this));

    mGizmoActions[0]->setShortcut(QKeySequence("Ctrl+Q"));
    mGizmoActions[1]->setShortcut(QKeySequence("Ctrl+W"));
    mGizmoActions[2]->setShortcut(QKeySequence("Ctrl+E"));
    mGizmoActions[3]->setShortcut(QKeySequence("Ctrl+R"));

    mpGizmoGroup = new QActionGroup(this);

    foreach (QAction *pAction, mGizmoActions)
    {
        pAction->setCheckable(true);
        mpGizmoGroup->addAction(pAction);
    }

    mGizmoActions[0]->setChecked(true);

    // Create transform combo box
    mpTransformCombo = new QComboBox(this);
    mpTransformCombo->addItem("World");
    mpTransformCombo->addItem("Local");

    // Connect signals and slots
    connect(mGizmoActions[0], SIGNAL(triggered()), this, SLOT(OnSelectObjectsTriggered()));
    connect(mGizmoActions[1], SIGNAL(triggered()), this, SLOT(OnTranslateTriggered()));
    connect(mGizmoActions[2], SIGNAL(triggered()), this, SLOT(OnRotateTriggered()));
    connect(mGizmoActions[3], SIGNAL(triggered()), this, SLOT(OnScaleTriggered()));
    connect(mpTransformCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnTransformSpaceChanged(int)));
    connect(this, SIGNAL(SelectionModified()), this, SLOT(OnSelectionModified()));
}

INodeEditor::~INodeEditor()
{
}

QUndoStack* INodeEditor::UndoStack()
{
    return &mUndoStack;
}

CScene* INodeEditor::Scene()
{
    return &mScene;
}

CGizmo* INodeEditor::Gizmo()
{
    return &mGizmo;
}

bool INodeEditor::IsGizmoVisible()
{
    return (mShowGizmo && !mSelection.empty());
}

void INodeEditor::BeginGizmoTransform()
{
    mGizmoTransforming = true;

    foreach (QAction *pAction, mGizmoActions)
        pAction->setEnabled(false);
}

void INodeEditor::EndGizmoTransform()
{
    mGizmoTransforming = false;

    foreach (QAction *pAction, mGizmoActions)
        pAction->setEnabled(true);

    if (mGizmo.Mode() == CGizmo::eTranslate)
        mUndoStack.push(CTranslateNodeCommand::End());
    else if (mGizmo.Mode() == CGizmo::eRotate)
        mUndoStack.push(CRotateNodeCommand::End());
    else if (mGizmo.Mode() == CGizmo::eScale)
        mUndoStack.push(CScaleNodeCommand::End());
}

ETransformSpace INodeEditor::CurrentTransformSpace()
{
    switch (mGizmo.Mode())
    {
    case CGizmo::eTranslate: return mTranslateSpace;
    case CGizmo::eRotate:    return mRotateSpace;
    case CGizmo::eScale:     return eLocalTransform;
    default:                 return eWorldTransform;
    }
}

void INodeEditor::RecalculateSelectionBounds()
{
    mSelectionBounds = CAABox::skInfinite;

    foreach (CSceneNode *pNode, mSelection)
        ExpandSelectionBounds(pNode);
}

void INodeEditor::ExpandSelectionBounds(CSceneNode *pNode)
{
    mSelectionBounds.ExpandBounds(pNode->AABox());

    if (pNode->NodeType() == eScriptNode)
    {
        CScriptNode *pScript = static_cast<CScriptNode*>(pNode);

        if (pScript->HasPreviewVolume())
            mSelectionBounds.ExpandBounds(pScript->PreviewVolumeAABox());
    }
}

void INodeEditor::SelectNode(CSceneNode *pNode)
{
    if (!pNode->IsSelected())
        mUndoStack.push(new CSelectNodeCommand(this, pNode, mSelection));
}

void INodeEditor::DeselectNode(CSceneNode *pNode)
{
    if (pNode->IsSelected())
        mUndoStack.push(new CDeselectNodeCommand(this, pNode, mSelection));
}

void INodeEditor::ClearSelection()
{
    if (!mSelection.empty())
        mUndoStack.push(new CClearSelectionCommand(this, mSelection));
}

void INodeEditor::ClearAndSelectNode(CSceneNode *pNode)
{
    if (mSelection.empty())
        mUndoStack.push(new CSelectNodeCommand(this, pNode, mSelection));

    else if ((mSelection.size() == 1) && (mSelection.front() == pNode))
        return;

    else
    {
        mUndoStack.beginMacro("Select");
        mUndoStack.push(new CClearSelectionCommand(this, mSelection));
        mUndoStack.push(new CSelectNodeCommand(this, pNode, mSelection));
        mUndoStack.endMacro();
    }
}

// ************ PUBLIC SLOTS ************
void INodeEditor::OnGizmoMoved()
{
    switch (mGizmo.Mode())
    {
        case CGizmo::eTranslate:
        {
            CVector3f delta = mGizmo.DeltaTranslation();
            mUndoStack.push(new CTranslateNodeCommand(this, mSelection, delta, mTranslateSpace));
            break;
        }

        case CGizmo::eRotate:
        {
            CQuaternion delta = mGizmo.DeltaRotation();
            mUndoStack.push(new CRotateNodeCommand(this, mSelection, CVector3f::skZero, delta, mRotateSpace));
            break;
        }

        case CGizmo::eScale:
        {
            CVector3f delta = mGizmo.DeltaScale();
            mUndoStack.push(new CScaleNodeCommand(this, mSelection, CVector3f::skZero, delta));
            break;
        }
    }

    RecalculateSelectionBounds();
    UpdateGizmoUI();
}

// ************ PRIVATE ************
void INodeEditor::UpdateTransformActionsEnabled()
{
    bool AllowTranslate = true, AllowRotate = true, AllowScale = true;
    bool SelectedModeWasEnabled = mpGizmoGroup->checkedAction()->isEnabled();

    for (auto it = mSelection.begin(); it != mSelection.end(); it++)
    {
        if (!(*it)->AllowsTranslate()) AllowTranslate = false;
        if (!(*it)->AllowsRotate()) AllowRotate = false;
        if (!(*it)->AllowsScale()) AllowScale = false;
    }

    mGizmoActions[1]->setEnabled(AllowTranslate);
    mGizmoActions[2]->setEnabled(AllowRotate);
    mGizmoActions[3]->setEnabled(AllowScale);

    bool SelectedModeIsEnabled = mpGizmoGroup->checkedAction()->isEnabled();

    if (SelectedModeWasEnabled && !SelectedModeIsEnabled)
        OnSelectObjectsTriggered();
    else if (SelectedModeIsEnabled && !SelectedModeWasEnabled)
        mpGizmoGroup->checkedAction()->trigger();
}

// ************ PRIVATE SLOTS ************
void INodeEditor::OnSelectObjectsTriggered()
{
    mGizmo.SetMode(CGizmo::eOff);
    mGizmo.SetTransformSpace(eWorldTransform);
    mShowGizmo = false;

    mpTransformCombo->setEnabled(false);
    mpTransformCombo->blockSignals(true);
    mpTransformCombo->setCurrentIndex(0);
    mpTransformCombo->blockSignals(false);

    GizmoModeChanged(CGizmo::eOff);
    UpdateGizmoUI();
}

void INodeEditor::OnTranslateTriggered()
{
    mGizmo.SetMode(CGizmo::eTranslate);
    mGizmo.SetTransformSpace(mTranslateSpace);
    mShowGizmo = true;

    mpTransformCombo->setEnabled(true);
    mpTransformCombo->blockSignals(true);
    mpTransformCombo->setCurrentIndex( (mTranslateSpace == eWorldTransform) ? 0 : 1 );
    mpTransformCombo->blockSignals(false);

    GizmoModeChanged(CGizmo::eTranslate);
    UpdateGizmoUI();
}

void INodeEditor::OnRotateTriggered()
{
    mGizmo.SetMode(CGizmo::eRotate);
    mGizmo.SetTransformSpace(mRotateSpace);
    mShowGizmo = true;

    mpTransformCombo->setEnabled(true);
    mpTransformCombo->blockSignals(true);
    mpTransformCombo->setCurrentIndex( (mRotateSpace == eWorldTransform) ? 0 : 1 );
    mpTransformCombo->blockSignals(false);

    GizmoModeChanged(CGizmo::eRotate);
    UpdateGizmoUI();
}

void INodeEditor::OnScaleTriggered()
{
    mGizmo.SetMode(CGizmo::eScale);
    mGizmo.SetTransformSpace(eLocalTransform);
    mShowGizmo = true;

    mpTransformCombo->setEnabled(false);
    mpTransformCombo->blockSignals(true);
    mpTransformCombo->setCurrentIndex(1);
    mpTransformCombo->blockSignals(false);

    GizmoModeChanged(CGizmo::eScale);
    UpdateGizmoUI();
}

void INodeEditor::OnTransformSpaceChanged(int spaceIndex)
{
    if ((mGizmo.Mode() == CGizmo::eScale) || (mGizmo.Mode() == CGizmo::eOff)) return;

    ETransformSpace space = (spaceIndex == 0 ? eWorldTransform : eLocalTransform);

    if (mGizmo.Mode() == CGizmo::eTranslate)
        mTranslateSpace = space;
    else
        mRotateSpace = space;

    mGizmo.SetTransformSpace(space);
}

void INodeEditor::OnSelectionModified()
{
    UpdateTransformActionsEnabled();
    UpdateSelectionUI();
}
