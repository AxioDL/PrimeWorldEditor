#include "INodeEditor.h"
#include "CSelectionIterator.h"
#include "Editor/Undo/UndoCommands.h"
#include <QMouseEvent>

INodeEditor::INodeEditor(QWidget *pParent)
    : IEditor(pParent)
    , mpSelection(new CNodeSelection)
{
    // Create gizmo actions
    mGizmoActions.append(new QAction(QIcon(QStringLiteral(":/icons/SelectMode.svg")), tr("Select Objects"), this));
    mGizmoActions.append(new QAction(QIcon(QStringLiteral(":/icons/Translate.svg")), tr("Translate"), this));
    mGizmoActions.append(new QAction(QIcon(QStringLiteral(":/icons/Rotate.svg")), tr("Rotate"), this));
    mGizmoActions.append(new QAction(QIcon(QStringLiteral(":/icons/Scale.svg")), tr("Scale"), this));

    mGizmoActions[0]->setShortcut(QKeySequence(Qt::Key_Control, Qt::Key_Q));
    mGizmoActions[1]->setShortcut(QKeySequence(Qt::Key_Control, Qt::Key_W));
    mGizmoActions[2]->setShortcut(QKeySequence(Qt::Key_Control, Qt::Key_E));
    mGizmoActions[3]->setShortcut(QKeySequence(Qt::Key_Control, Qt::Key_R));

    mpGizmoGroup = new QActionGroup(this);

    for (QAction *pAction : mGizmoActions)
    {
        pAction->setCheckable(true);
        mpGizmoGroup->addAction(pAction);
    }

    mGizmoActions[0]->setChecked(true);

    // Create transform combo box
    mpTransformCombo = new QComboBox(this);
    mpTransformCombo->addItem(tr("World"));
    mpTransformCombo->addItem(tr("Local"));

    // Connect signals and slots
    connect(mGizmoActions[0], &QAction::triggered, this, &INodeEditor::OnSelectObjectsTriggered);
    connect(mGizmoActions[1], &QAction::triggered, this, &INodeEditor::OnTranslateTriggered);
    connect(mGizmoActions[2], &QAction::triggered, this, &INodeEditor::OnRotateTriggered);
    connect(mGizmoActions[3], &QAction::triggered, this, &INodeEditor::OnScaleTriggered);
    connect(mpTransformCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &INodeEditor::OnTransformSpaceChanged);
    connect(mpSelection, &CNodeSelection::Modified, this, &INodeEditor::OnSelectionModified);
}

INodeEditor::~INodeEditor()
{
    delete mpSelection;
}

CScene* INodeEditor::Scene()
{
    return &mScene;
}

CGizmo* INodeEditor::Gizmo()
{
    return &mGizmo;
}

bool INodeEditor::IsGizmoVisible() const
{
    return (mShowGizmo && !mpSelection->IsEmpty());
}

void INodeEditor::BeginGizmoTransform()
{
    mGizmoTransforming = true;
    if ((qApp->keyboardModifiers() & Qt::ShiftModifier) != 0)
        mCloneState = ECloneState::ReadyToClone;

    for (QAction *pAction : mGizmoActions)
        pAction->setEnabled(false);
}

void INodeEditor::EndGizmoTransform()
{
    mGizmoTransforming = false;

    for (QAction *pAction : mGizmoActions)
        pAction->setEnabled(true);

    if (mGizmo.HasTransformed())
    {
        if (mGizmo.Mode() == CGizmo::EGizmoMode::Translate)
            mUndoStack.push(CTranslateNodeCommand::End());
        else if (mGizmo.Mode() == CGizmo::EGizmoMode::Rotate)
            mUndoStack.push(CRotateNodeCommand::End());
        else if (mGizmo.Mode() == CGizmo::EGizmoMode::Scale)
            mUndoStack.push(CScaleNodeCommand::End());
    }

    if (mCloneState == ECloneState::Cloning)
        mUndoStack.endMacro();

    mCloneState = ECloneState::NotCloning;
}

ETransformSpace INodeEditor::CurrentTransformSpace() const
{
    switch (mGizmo.Mode())
    {
    case CGizmo::EGizmoMode::Translate: return mTranslateSpace;
    case CGizmo::EGizmoMode::Rotate:    return mRotateSpace;
    case CGizmo::EGizmoMode::Scale:     return ETransformSpace::Local;
    default:                            return ETransformSpace::World;
    }
}

void INodeEditor::SelectNode(CSceneNode *pNode)
{
    if (mSelectionLocked)
        return;

    if (!pNode->IsSelected())
        mUndoStack.push(new CSelectNodeCommand(mpSelection, pNode));
}

void INodeEditor::BatchSelectNodes(QList<CSceneNode*> Nodes, bool ClearExistingSelection, const QString& rkCommandName /*= "Select"*/)
{
    if (mSelectionLocked)
        return;

    if (!ClearExistingSelection)
    {
        for (CSceneNode *pNode : Nodes)
        {
            if (pNode->IsSelected())
                Nodes.removeOne(pNode);
        }
    }

    if (Nodes.size() > 0)
    {
        mUndoStack.beginMacro(rkCommandName);

        if (ClearExistingSelection)
            ClearSelection();

        for (CSceneNode *pNode : Nodes)
            SelectNode(pNode);

        mUndoStack.endMacro();
    }
}

void INodeEditor::DeselectNode(CSceneNode *pNode)
{
    if (mSelectionLocked)
        return;

    if (pNode->IsSelected())
        mUndoStack.push(new CDeselectNodeCommand(mpSelection, pNode));
}

void INodeEditor::BatchDeselectNodes(QList<CSceneNode*> Nodes, const QString& rkCommandName /*= "Deselect"*/)
{
    if (mSelectionLocked)
        return;

    for (CSceneNode *pNode : Nodes)
    {
        if (!pNode->IsSelected())
            Nodes.removeOne(pNode);
    }

    if (Nodes.size() > 0)
    {
        mUndoStack.beginMacro(rkCommandName);

        for (CSceneNode *pNode : Nodes)
            DeselectNode(pNode);

        mUndoStack.endMacro();
    }
}

void INodeEditor::ClearSelection()
{
    if (mSelectionLocked)
        return;

    if (!mpSelection->IsEmpty())
        mUndoStack.push(new CClearSelectionCommand(mpSelection));
}

void INodeEditor::ClearAndSelectNode(CSceneNode *pNode)
{
    if (mSelectionLocked)
        return;

    if (mpSelection->IsEmpty())
    {
        mUndoStack.push(new CSelectNodeCommand(mpSelection, pNode));
    }
    else if ((mpSelection->Size() == 1) && (mpSelection->Front() == pNode))
    {
        return;
    }
    else
    {
        mUndoStack.beginMacro(tr("Select"));
        mUndoStack.push(new CClearSelectionCommand(mpSelection));
        mUndoStack.push(new CSelectNodeCommand(mpSelection, pNode));
        mUndoStack.endMacro();
    }
}

void INodeEditor::SelectAll(FNodeFlags NodeFlags)
{
    if (!mSelectionLocked)
        mUndoStack.push(new CSelectAllCommand(mpSelection, &mScene, NodeFlags));
}

void INodeEditor::InvertSelection(FNodeFlags NodeFlags)
{
    if (!mSelectionLocked)
        mUndoStack.push(new CInvertSelectionCommand(mpSelection, &mScene, NodeFlags));
}

void INodeEditor::SetSelectionLocked(bool Locked)
{
    mSelectionLocked = Locked;
}

bool INodeEditor::HasSelection() const
{
    return (!mpSelection->IsEmpty());
}

CNodeSelection* INodeEditor::Selection() const
{
    return mpSelection;
}

void INodeEditor::EnterPickMode(FNodeFlags AllowedNodes, bool ExitOnInvalidPick, bool EmitOnInvalidPick, bool EmitHoverOnButtonPress, QCursor Cursor /*= Qt::CrossCursor*/)
{
    // If we're already in pick mode, exit first so the previous caller has a chance to disconnect
    if (mPickMode)
        ExitPickMode();

    mPickMode = true;
    mAllowedPickNodes = AllowedNodes;
    mExitOnInvalidPick = ExitOnInvalidPick;
    mEmitOnInvalidPick = EmitOnInvalidPick;
    mEmitOnButtonPress = EmitHoverOnButtonPress;
    emit PickModeEntered(Cursor);
}

void INodeEditor::ExitPickMode()
{
    if (!mPickMode)
        return;

    mPickMode = false;
    emit PickModeExited();
}

void INodeEditor::NotifySelectionTransformed()
{
    for (CSelectionIterator It(mpSelection); It; ++It)
        It->OnTransformed();

    mpSelection->UpdateBounds();
    emit SelectionTransformed();
}

void INodeEditor::NotifyNodeAboutToBeSpawned()
{
    emit NodeAboutToBeSpawned();
}

void INodeEditor::NotifyNodeSpawned(CSceneNode *pNode)
{
    emit NodeSpawned(pNode);
}

void INodeEditor::NotifyNodeAboutToBeDeleted(CSceneNode *pNode)
{
    emit NodeAboutToBeDeleted(pNode);
}

void INodeEditor::NotifyNodeDeleted()
{
    emit NodeDeleted();
}

// ************ PUBLIC SLOTS ************
void INodeEditor::OnSelectionModified()
{
    UpdateSelectionUI();
    emit SelectionModified();
}

void INodeEditor::OnGizmoMoved()
{
    if (mCloneState == ECloneState::ReadyToClone)
    {
        mUndoStack.beginMacro(tr("Clone"));
        mUndoStack.push(new CCloneSelectionCommand(this));
        mCloneState = ECloneState::Cloning;
    }

    switch (mGizmo.Mode())
    {
        case CGizmo::EGizmoMode::Translate:
        {
            CVector3f Delta = mGizmo.DeltaTranslation();
            mUndoStack.push(new CTranslateNodeCommand(this, mpSelection->SelectedNodeList(), Delta, mTranslateSpace));
            break;
        }

        case CGizmo::EGizmoMode::Rotate:
        {
            CQuaternion Delta = mGizmo.DeltaRotation();
            mUndoStack.push(new CRotateNodeCommand(this, mpSelection->SelectedNodeList(), true, mGizmo.Position(), mGizmo.Rotation(), Delta, mRotateSpace));
            break;
        }

        case CGizmo::EGizmoMode::Scale:
        {
            CVector3f Delta = mGizmo.DeltaScale();
            mUndoStack.push(new CScaleNodeCommand(this, mpSelection->SelectedNodeList(), true, mGizmo.Position(), Delta));
            break;
        }
        default: break;
    }

    UpdateGizmoUI();
}

// ************ PROTECTED SLOTS ************
void INodeEditor::OnViewportClick(const SRayIntersection& rkRayIntersect, QMouseEvent *pEvent)
{
    CSceneNode *pNode = rkRayIntersect.pNode;

    // Not in pick mode: process node selection/deselection
    if (!mPickMode)
    {
        bool ValidNode = (pNode && mpSelection->IsAllowedType(pNode));
        bool AltPressed = ((pEvent->modifiers() & Qt::AltModifier) != 0);
        bool CtrlPressed = ((pEvent->modifiers() & Qt::ControlModifier) != 0);

        // Alt: Deselect
        if (AltPressed)
        {
            if (!ValidNode)
                return;

            DeselectNode(pNode);
        }

        // Ctrl: Add to selection
        else if (CtrlPressed)
        {
            if (ValidNode)
                SelectNode(pNode);
        }

        // Neither: clear selection + select
        else
        {
            if (!mGizmoHovering)
            {
                if (ValidNode)
                    ClearAndSelectNode(pNode);
                else
                    ClearSelection();
            }
        }
    }

    // In pick mode: process node pick
    else
    {
        bool ValidNode = (pNode && (pNode->NodeType() & mAllowedPickNodes));

        if (ValidNode || mEmitOnInvalidPick)
            emit PickModeClick(rkRayIntersect, pEvent);

        if (!ValidNode && mExitOnInvalidPick)
            ExitPickMode();
    }
}

void INodeEditor::OnViewportInputProcessed(const SRayIntersection& rkRayIntersect, QMouseEvent *pEvent)
{
    // In pick mode: process node hover
    if (mPickMode)
    {
        CSceneNode *pNode = rkRayIntersect.pNode;
        bool NewNode = pNode != mpPickHoverNode;

        bool ButtonsChanged = mPickButtons != pEvent->buttons();
        bool ModifiersChanged = mPickModifiers != pEvent->modifiers();

        if (NewNode || ((ModifiersChanged || ButtonsChanged) && mEmitOnButtonPress))
        {
            bool ValidNode = (pNode && (pNode->NodeType() & mAllowedPickNodes));

            if (ValidNode || mEmitOnInvalidPick)
                emit PickModeHoverChanged(rkRayIntersect, pEvent);
            else
                emit PickModeHoverChanged(SRayIntersection(), pEvent);
        }

        mpPickHoverNode = rkRayIntersect.pNode;
        mPickButtons = pEvent->buttons();
        mPickModifiers = pEvent->modifiers();
    }
}

// ************ PRIVATE ************
void INodeEditor::UpdateTransformActionsEnabled()
{
    bool AllowTranslate = true, AllowRotate = true, AllowScale = true;
    bool SelectedModeWasEnabled = mpGizmoGroup->checkedAction()->isEnabled();

    for (CSelectionIterator It(mpSelection); It; ++It)
    {
        if (!It->AllowsTranslate()) AllowTranslate = false;
        if (!It->AllowsRotate())    AllowRotate = false;
        if (!It->AllowsScale())     AllowScale = false;
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
    mGizmo.SetMode(CGizmo::EGizmoMode::Off);
    mGizmo.SetTransformSpace(ETransformSpace::World);
    mShowGizmo = false;

    mpTransformCombo->setEnabled(false);
    mpTransformCombo->blockSignals(true);
    mpTransformCombo->setCurrentIndex(0);
    mpTransformCombo->blockSignals(false);

    GizmoModeChanged(CGizmo::EGizmoMode::Off);
    UpdateGizmoUI();
}

void INodeEditor::OnTranslateTriggered()
{
    mGizmo.SetMode(CGizmo::EGizmoMode::Translate);
    mGizmo.SetTransformSpace(mTranslateSpace);
    mShowGizmo = true;

    mpTransformCombo->setEnabled(true);
    mpTransformCombo->blockSignals(true);
    mpTransformCombo->setCurrentIndex( (mTranslateSpace == ETransformSpace::World) ? 0 : 1 );
    mpTransformCombo->blockSignals(false);

    GizmoModeChanged(CGizmo::EGizmoMode::Translate);
    UpdateGizmoUI();
}

void INodeEditor::OnRotateTriggered()
{
    mGizmo.SetMode(CGizmo::EGizmoMode::Rotate);
    mGizmo.SetTransformSpace(mRotateSpace);
    mShowGizmo = true;

    mpTransformCombo->setEnabled(true);
    mpTransformCombo->blockSignals(true);
    mpTransformCombo->setCurrentIndex( (mRotateSpace == ETransformSpace::World) ? 0 : 1 );
    mpTransformCombo->blockSignals(false);

    GizmoModeChanged(CGizmo::EGizmoMode::Rotate);
    UpdateGizmoUI();
}

void INodeEditor::OnScaleTriggered()
{
    mGizmo.SetMode(CGizmo::EGizmoMode::Scale);
    mGizmo.SetTransformSpace(ETransformSpace::Local);
    mShowGizmo = true;

    mpTransformCombo->setEnabled(false);
    mpTransformCombo->blockSignals(true);
    mpTransformCombo->setCurrentIndex(1);
    mpTransformCombo->blockSignals(false);

    GizmoModeChanged(CGizmo::EGizmoMode::Scale);
    UpdateGizmoUI();
}

void INodeEditor::OnTransformSpaceChanged(int SpaceIndex)
{
    if ((mGizmo.Mode() == CGizmo::EGizmoMode::Scale) || (mGizmo.Mode() == CGizmo::EGizmoMode::Off)) return;

    ETransformSpace Space = (SpaceIndex == 0 ? ETransformSpace::World : ETransformSpace::Local);

    if (mGizmo.Mode() == CGizmo::EGizmoMode::Translate)
        mTranslateSpace = Space;
    else
        mRotateSpace = Space;

    mGizmo.SetTransformSpace(Space);
}
