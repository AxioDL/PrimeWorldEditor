#include "INodeEditor.h"
#include "CSelectionIterator.h"
#include "Editor/Undo/UndoCommands.h"
#include <QMouseEvent>

INodeEditor::INodeEditor(QWidget *pParent)
    : QMainWindow(pParent)
    , mPickMode(false)
    , mpSelection(new CNodeSelection)
    , mSelectionLocked(false)
    , mShowGizmo(false)
    , mGizmoHovering(false)
    , mGizmoTransforming(false)
    , mTranslateSpace(eWorldTransform)
    , mRotateSpace(eWorldTransform)
    , mCloneState(eNotCloning)
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
    connect(mpSelection, SIGNAL(Modified()), this, SLOT(OnSelectionModified()));
}

INodeEditor::~INodeEditor()
{
    delete mpSelection;
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
    return (mShowGizmo && !mpSelection->IsEmpty());
}

void INodeEditor::BeginGizmoTransform()
{
    mGizmoTransforming = true;
    if ((qApp->keyboardModifiers() & Qt::ShiftModifier) != 0) mCloneState = eReadyToClone;

    foreach (QAction *pAction, mGizmoActions)
        pAction->setEnabled(false);
}

void INodeEditor::EndGizmoTransform()
{
    mGizmoTransforming = false;

    foreach (QAction *pAction, mGizmoActions)
        pAction->setEnabled(true);

    if (mGizmo.HasTransformed())
    {
        if (mGizmo.Mode() == CGizmo::eTranslate)
            mUndoStack.push(CTranslateNodeCommand::End());
        else if (mGizmo.Mode() == CGizmo::eRotate)
            mUndoStack.push(CRotateNodeCommand::End());
        else if (mGizmo.Mode() == CGizmo::eScale)
            mUndoStack.push(CScaleNodeCommand::End());
    }

    if (mCloneState == eCloning)
        mUndoStack.endMacro();

    mCloneState = eNotCloning;
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

void INodeEditor::SelectNode(CSceneNode *pNode)
{
    if (!mSelectionLocked)
    {
        if (!pNode->IsSelected())
            mUndoStack.push(new CSelectNodeCommand(mpSelection, pNode));
    }
}

void INodeEditor::DeselectNode(CSceneNode *pNode)
{
    if (!mSelectionLocked)
    {
        if (pNode->IsSelected())
            mUndoStack.push(new CDeselectNodeCommand(mpSelection, pNode));
    }
}

void INodeEditor::ClearSelection()
{
    if (!mSelectionLocked)
    {
        if (!mpSelection->IsEmpty())
            mUndoStack.push(new CClearSelectionCommand(mpSelection));
    }
}

void INodeEditor::ClearAndSelectNode(CSceneNode *pNode)
{
    if (!mSelectionLocked)
    {
        if (mpSelection->IsEmpty())
            mUndoStack.push(new CSelectNodeCommand(mpSelection, pNode));

        else if ((mpSelection->Size() == 1) && (mpSelection->Front() == pNode))
            return;

        else
        {
            mUndoStack.beginMacro("Select");
            mUndoStack.push(new CClearSelectionCommand(mpSelection));
            mUndoStack.push(new CSelectNodeCommand(mpSelection, pNode));
            mUndoStack.endMacro();
        }
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
    if (mPickMode)
    {
        mPickMode = false;
        emit PickModeExited();
    }
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
    if (mCloneState == eReadyToClone)
    {
        mUndoStack.beginMacro("Clone");
        mUndoStack.push(new CCloneSelectionCommand(this));
        mCloneState = eCloning;
    }

    switch (mGizmo.Mode())
    {
        case CGizmo::eTranslate:
        {
            CVector3f delta = mGizmo.DeltaTranslation();
            mUndoStack.push(new CTranslateNodeCommand(this, mpSelection->SelectedNodeList(), delta, mTranslateSpace));
            break;
        }

        case CGizmo::eRotate:
        {
            CQuaternion delta = mGizmo.DeltaRotation();
            mUndoStack.push(new CRotateNodeCommand(this, mpSelection->SelectedNodeList(), CVector3f::skZero, delta, mRotateSpace));
            break;
        }

        case CGizmo::eScale:
        {
            CVector3f delta = mGizmo.DeltaScale();
            mUndoStack.push(new CScaleNodeCommand(this, mpSelection->SelectedNodeList(), CVector3f::skZero, delta));
            break;
        }
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
