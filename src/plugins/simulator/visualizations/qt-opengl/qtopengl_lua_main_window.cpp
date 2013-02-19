#include "qtopengl_lua_main_window.h"
#include "qtopengl_lua_editor.h"
#include "qtopengl_lua_find_dialog.h"
#include "qtopengl_lua_syntax_highlighter.h"
#include "qtopengl_main_window.h"
#include "qtopengl_widget.h"

#include <argos3/core/wrappers/lua/lua_controller.h>
#include <argos3/core/simulator/simulator.h>
#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/entity/composable_entity.h>
#include <argos3/core/simulator/entity/controllable_entity.h>

#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QTextStream>
#include <QToolBar>
#include <QTableWidget>

namespace argos {

   /****************************************/
   /****************************************/

   CQTOpenGLLuaMainWindow::CQTOpenGLLuaMainWindow(CQTOpenGLMainWindow* pc_parent) :
      QMainWindow(pc_parent),
      m_pcMainWindow(pc_parent),
      m_pcStatusbar(NULL),
      m_pcCodeEditor(NULL),
      m_pcFindDialog(NULL),
      m_pcLuaMessageTable(NULL) {
      /* Add a status bar */
      m_pcStatusbar = new QStatusBar(this);
      setStatusBar(m_pcStatusbar);
      /* Create the Lua message table */
      CreateLuaMessageTable();
      /* Populate list of Lua controllers */
      PopulateLuaControllers();
      /* Create editor */
      CreateCodeEditor();
      /* Create actions */
      CreateFileActions();
      CreateEditActions();
      CreateCodeActions();
      /* Set empty file */
      SetCurrentFile("");
      /* Read settings */
      ReadSettings();
   }

   /****************************************/
   /****************************************/

   CQTOpenGLLuaMainWindow::~CQTOpenGLLuaMainWindow() {
      WriteSettings();
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::New() {
      if(MaybeSave()) {
         m_pcCodeEditor->clear();
         SetCurrentFile("");
      }
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::Open() {
      if(MaybeSave()) {
         QString strNewFileName =
            QFileDialog::getOpenFileName(this,
                                         tr("Open File"),
                                         "",
                                         "Lua Files (*.lua)");
         if (!strNewFileName.isEmpty()) {
            OpenFile(strNewFileName);
         }
      }
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::OpenRecentFile() {
      QAction* pcAction = qobject_cast<QAction*>(sender());
      if (pcAction) {
         OpenFile(pcAction->data().toString());
      }
   }

   /****************************************/
   /****************************************/

   bool CQTOpenGLLuaMainWindow::Save() {
      if (m_strFileName.isEmpty()) {
         return SaveAs();
      } else {
         return SaveFile(m_strFileName);
      }
   }

   /****************************************/
   /****************************************/

   bool CQTOpenGLLuaMainWindow::SaveAs() {
      QString strNewFileName =
         QFileDialog::getSaveFileName(this,
                                      tr("Save File"),
                                      "",
                                      "Lua Files (*.lua)");
      if (strNewFileName.isEmpty())
         return false;
      return SaveFile(strNewFileName);
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::Execute() {
      if(MaybeSave()) {
         QApplication::setOverrideCursor(Qt::WaitCursor);
         for(size_t i = 0; i < m_vecControllers.size(); ++i) {
            m_vecControllers[i]->SetLuaScript(m_strFileName.toStdString());
         }
         QApplication::restoreOverrideCursor();
         statusBar()->showMessage(tr("Execution started"), 2000);
      }
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::Find() {
      if(! m_pcFindDialog) {
         m_pcFindDialog = new CQTOpenGLLuaFindDialog(this);
      }
      m_pcFindDialog->show();
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::CodeModified() {
      setWindowModified(m_pcCodeEditor->document()->isModified());
   }

   /****************************************/
   /****************************************/

   bool CQTOpenGLLuaMainWindow::MaybeSave() {
      if(m_pcCodeEditor->document()->isModified()) {
         QMessageBox::StandardButton tReply;
         tReply = QMessageBox::warning(this, tr("ARGoS v3.0 - Lua Editor"),
                                       tr("The document has been modified.\n"
                                          "Do you want to save your changes?"),
                                       QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
         if (tReply == QMessageBox::Save)
            return Save();
         else if (tReply == QMessageBox::Cancel)
            return false;
      }
      return true;
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::PopulateLuaControllers() {
      CSpace& cSpace = CSimulator::GetInstance().GetSpace();
      CSpace::TMapPerType& tControllables = cSpace.GetEntitiesByType("controller");
      for(CSpace::TMapPerType::iterator it = tControllables.begin();
          it != tControllables.end();
          ++it) {
         CControllableEntity* pcControllable = any_cast<CControllableEntity*>(it->second);
         CLuaController* pcLuaController = dynamic_cast<CLuaController*>(&(pcControllable->GetController()));
         if(pcLuaController) {
            m_vecControllers.push_back(pcLuaController);
         }
         else {
            LOGERR << "[WARNING] Entity \""
                   << pcControllable->GetParent().GetId()
                   << "\" does not have a Lua controller associated"
                   << std::endl;
         }
      }
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::ReadSettings() {
      QSettings cSettings;
      cSettings.beginGroup("LuaEditor");
      resize(cSettings.value("size", QSize(640,480)).toSize());
      move(cSettings.value("position", QPoint(0,0)).toPoint());
      cSettings.endGroup();
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::WriteSettings() {
      QSettings cSettings;
      cSettings.beginGroup("LuaEditor");
      cSettings.setValue("size", size());
      cSettings.setValue("position", pos());
      cSettings.endGroup();
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::CreateCodeEditor() {
      QFont cFont;
      cFont.setFamily("Courier");
      cFont.setFixedPitch(true);
      cFont.setPointSize(10);
      m_pcCodeEditor = new CQTOpenGLLuaEditor(this);
      m_pcCodeEditor->setFont(cFont);
      new CQTOpenGLLuaSyntaxHighlighter(m_pcCodeEditor->document());
      setCentralWidget(m_pcCodeEditor);
      connect(m_pcCodeEditor->document(), SIGNAL(contentsChanged()),
              this, SLOT(CodeModified()));
      connect(&(m_pcMainWindow->GetOpenGLWidget()), SIGNAL(StepDone(int)),
              this, SLOT(CheckLuaStatus(int)));
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::CreateLuaMessageTable() {
      QDockWidget* pcLuaMsgDock = new QDockWidget(tr("Messages"), this);
      pcLuaMsgDock->setObjectName("LuaMessageDock");
      m_pcLuaMessageTable = new QTableWidget();
      m_pcLuaMessageTable->setColumnCount(3);
      QStringList listHeaders;
      listHeaders << tr("Robot")
                  << tr("Line")
                  << tr("Message");
      m_pcLuaMessageTable->setHorizontalHeaderLabels(listHeaders);
      m_pcLuaMessageTable->horizontalHeader()->setStretchLastSection(true);
      m_pcLuaMessageTable->setSelectionBehavior(QAbstractItemView::SelectRows);
      m_pcLuaMessageTable->setSelectionMode(QAbstractItemView::SingleSelection);
      pcLuaMsgDock->setWidget(m_pcLuaMessageTable);
      addDockWidget(Qt::BottomDockWidgetArea, pcLuaMsgDock);
      connect(m_pcLuaMessageTable, SIGNAL(itemSelectionChanged()),
              this, SLOT(HandleMsgTableSelection()));
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::CreateFileActions() {
      QIcon cFileNewIcon;
      cFileNewIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/new.png"));
      m_pcFileNewAction = new QAction(cFileNewIcon, tr("&New"), this);
      m_pcFileNewAction->setToolTip(tr("Create a new file"));
      m_pcFileNewAction->setStatusTip(tr("Create a new file"));
      m_pcFileNewAction->setShortcut(QKeySequence::New);
      connect(m_pcFileNewAction, SIGNAL(triggered()),
              this, SLOT(New()));
      QIcon cFileOpenIcon;
      cFileOpenIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/open.png"));
      m_pcFileOpenAction = new QAction(cFileOpenIcon, tr("&Open..."), this);
      m_pcFileOpenAction->setToolTip(tr("Open a file"));
      m_pcFileOpenAction->setStatusTip(tr("Open a file"));
      m_pcFileOpenAction->setShortcut(QKeySequence::Open);
      connect(m_pcFileOpenAction, SIGNAL(triggered()),
              this, SLOT(Open()));
      for (int i = 0; i < MAX_RECENT_FILES; ++i) {
         m_pcFileOpenRecentAction[i] = new QAction(this);
         m_pcFileOpenRecentAction[i]->setVisible(false);
         connect(m_pcFileOpenRecentAction[i], SIGNAL(triggered()),
                 this, SLOT(OpenRecentFile()));
      }
      QIcon cFileSaveIcon;
      cFileSaveIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/save.png"));
      m_pcFileSaveAction = new QAction(cFileSaveIcon, tr("&Save"), this);
      m_pcFileSaveAction->setToolTip(tr("Save the current file"));
      m_pcFileSaveAction->setStatusTip(tr("Save the current file"));
      m_pcFileSaveAction->setShortcut(QKeySequence::Save);
      connect(m_pcFileSaveAction, SIGNAL(triggered()),
              this, SLOT(Save()));
      QIcon cFileSaveAsIcon;
      cFileSaveAsIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/saveas.png"));
      m_pcFileSaveAsAction = new QAction(cFileSaveAsIcon, tr("S&ave as..."), this);
      m_pcFileSaveAsAction->setToolTip(tr("Save the current file under a new name"));
      m_pcFileSaveAsAction->setStatusTip(tr("Save the current file under a new name"));
      m_pcFileSaveAsAction->setShortcut(QKeySequence::SaveAs);
      connect(m_pcFileSaveAsAction, SIGNAL(triggered()),
              this, SLOT(SaveAs()));
      QMenu* pcMenu = menuBar()->addMenu(tr("&File"));
      pcMenu->addAction(m_pcFileNewAction);
      pcMenu->addSeparator();
      pcMenu->addAction(m_pcFileOpenAction);
      pcMenu->addSeparator();
      pcMenu->addAction(m_pcFileSaveAction);
      pcMenu->addAction(m_pcFileSaveAsAction);
      m_pcFileSeparateRecentAction = pcMenu->addSeparator();
      for (int i = 0; i < MAX_RECENT_FILES; ++i) {
         pcMenu->addAction(m_pcFileOpenRecentAction[i]);
      }
      QToolBar* pcToolBar = addToolBar(tr("File"));
      pcToolBar->addAction(m_pcFileNewAction);
      pcToolBar->addAction(m_pcFileOpenAction);
      pcToolBar->addAction(m_pcFileSaveAction);
      UpdateRecentFiles();
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::CreateEditActions() {
      QIcon cEditUndoIcon;
      cEditUndoIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/undo.png"));
      m_pcEditUndoAction = new QAction(cEditUndoIcon, tr("&Undo"), this);
      m_pcEditUndoAction->setToolTip(tr("Undo last operation"));
      m_pcEditUndoAction->setStatusTip(tr("Undo last operation"));
      m_pcEditUndoAction->setShortcut(QKeySequence::Undo);
      connect(m_pcEditUndoAction, SIGNAL(triggered()),
              m_pcCodeEditor, SLOT(undo()));
      QIcon cEditRedoIcon;
      cEditRedoIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/redo.png"));
      m_pcEditRedoAction = new QAction(cEditRedoIcon, tr("&Redo"), this);
      m_pcEditRedoAction->setToolTip(tr("Redo last operation"));
      m_pcEditRedoAction->setStatusTip(tr("Redo last operation"));
      m_pcEditRedoAction->setShortcut(QKeySequence::Redo);
      connect(m_pcEditRedoAction, SIGNAL(triggered()),
              m_pcCodeEditor, SLOT(redo()));
      QIcon cEditCopyIcon;
      cEditCopyIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/copy.png"));
      m_pcEditCopyAction = new QAction(cEditCopyIcon, tr("&Copy"), this);
      m_pcEditCopyAction->setToolTip(tr("Copy selected text into clipboard"));
      m_pcEditCopyAction->setStatusTip(tr("Copy selected text into clipboard"));
      m_pcEditCopyAction->setShortcut(QKeySequence::Copy);
      connect(m_pcEditCopyAction, SIGNAL(triggered()),
              m_pcCodeEditor, SLOT(copy()));
      QIcon cEditCutIcon;
      cEditCutIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/cut.png"));
      m_pcEditCutAction = new QAction(cEditCutIcon, tr("&Cut"), this);
      m_pcEditCutAction->setToolTip(tr("Move selected text into clipboard"));
      m_pcEditCutAction->setStatusTip(tr("Move selected text into clipboard"));
      m_pcEditCutAction->setShortcut(QKeySequence::Cut);
      connect(m_pcEditCutAction, SIGNAL(triggered()),
              m_pcCodeEditor, SLOT(cut()));
      QIcon cEditPasteIcon;
      cEditPasteIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/paste.png"));
      m_pcEditPasteAction = new QAction(cEditPasteIcon, tr("&Paste"), this);
      m_pcEditPasteAction->setToolTip(tr("Paste text from clipboard"));
      m_pcEditPasteAction->setStatusTip(tr("Paste text from clipboard"));
      m_pcEditPasteAction->setShortcut(QKeySequence::Paste);
      connect(m_pcEditPasteAction, SIGNAL(triggered()),
              m_pcCodeEditor, SLOT(paste()));
      QIcon cEditFindIcon;
      cEditFindIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/find.png"));
      m_pcEditFindAction = new QAction(cEditFindIcon, tr("&Find/Replace"), this);
      m_pcEditFindAction->setToolTip(tr("Find/replace text"));
      m_pcEditFindAction->setStatusTip(tr("Find/replace text"));
      m_pcEditFindAction->setShortcut(QKeySequence::Find);
      connect(m_pcEditFindAction, SIGNAL(triggered()),
              this, SLOT(Find()));
      QMenu* pcMenu = menuBar()->addMenu(tr("&Edit"));
      pcMenu->addAction(m_pcEditUndoAction);
      pcMenu->addAction(m_pcEditRedoAction);
      pcMenu->addSeparator();
      pcMenu->addAction(m_pcEditCopyAction);
      pcMenu->addAction(m_pcEditCutAction);
      pcMenu->addAction(m_pcEditPasteAction);
      pcMenu->addSeparator();
      pcMenu->addAction(m_pcEditFindAction);
      QToolBar* pcToolBar = addToolBar(tr("Edit"));
      pcToolBar->addAction(m_pcEditUndoAction);
      pcToolBar->addAction(m_pcEditRedoAction);
      pcToolBar->addSeparator();
      pcToolBar->addAction(m_pcEditCopyAction);
      pcToolBar->addAction(m_pcEditCutAction);
      pcToolBar->addAction(m_pcEditPasteAction);
      pcToolBar->addAction(m_pcEditFindAction);
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::CreateCodeActions() {
      QIcon cCodeExecuteIcon;
      cCodeExecuteIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/execute.png"));
      m_pcCodeExecuteAction = new QAction(cCodeExecuteIcon, tr("&Execute"), this);
      m_pcCodeExecuteAction->setToolTip(tr("Execute code"));
      m_pcCodeExecuteAction->setStatusTip(tr("Execute code"));
      m_pcCodeExecuteAction->setShortcut(tr("Ctrl+E"));
      connect(m_pcCodeExecuteAction, SIGNAL(triggered()),
              this, SLOT(Execute()));
      QMenu* pcMenu = menuBar()->addMenu(tr("&Code"));
      pcMenu->addAction(m_pcCodeExecuteAction);
      QToolBar* pcToolBar = addToolBar(tr("Code"));
      pcToolBar->addAction(m_pcCodeExecuteAction);
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::OpenFile(const QString& str_path) {
      QFile cFile(str_path);
      if(! cFile.open(QFile::ReadOnly | QFile::Text)) {
         QMessageBox::warning(this, tr("ARGoS v3.0 - Lua Editor"),
                              tr("Cannot read file %1:\n%2.")
                              .arg(str_path)
                              .arg(cFile.errorString()));
         return;
      }
      QApplication::setOverrideCursor(Qt::WaitCursor);
      m_pcCodeEditor->setPlainText(cFile.readAll());
      QApplication::restoreOverrideCursor();
      SetCurrentFile(str_path);
      statusBar()->showMessage(tr("File loaded"), 2000);
   }

   /****************************************/
   /****************************************/

   bool CQTOpenGLLuaMainWindow::SaveFile(const QString& str_path) {
      QFile cFile(str_path);
      if(! cFile.open(QFile::WriteOnly | QFile::Text)) {
         QMessageBox::warning(this, tr("ARGoS v3.0 - Lua Editor"),
                              tr("Cannot write file %1:\n%2.")
                              .arg(str_path)
                              .arg(cFile.errorString()));
         return false;
      }
      QTextStream cOut(&cFile);
      QApplication::setOverrideCursor(Qt::WaitCursor);
      cOut << m_pcCodeEditor->toPlainText();
      QApplication::restoreOverrideCursor();
      SetCurrentFile(str_path);
      statusBar()->showMessage(tr("File saved"), 2000);
      return true;
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::SetCurrentFile(const QString& str_path) {
      m_strFileName = str_path;
      m_pcCodeEditor->document()->setModified(false);
      setWindowModified(false);
      QString strShownName;
      if(m_strFileName.isEmpty()) {
         strShownName = "untitled";
      }
      else {
         strShownName = StrippedFileName(m_strFileName);
      }
      setWindowTitle(tr("%1[*] - ARGoS v3.0 - Lua Editor").arg(strShownName));
      if(!m_strFileName.isEmpty()) {
         QSettings cSettings;
         cSettings.beginGroup("LuaEditor");
         QStringList listFiles = cSettings.value("recent_files").toStringList();
         listFiles.removeAll(m_strFileName);
         listFiles.prepend(m_strFileName);
         while(listFiles.size() > MAX_RECENT_FILES) {
            listFiles.removeLast();
         }
         cSettings.setValue("recent_files", listFiles);
         cSettings.endGroup();
         UpdateRecentFiles();
      }
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::CheckLuaStatus(int n_step) {
      static std::vector<std::string> vecFields;
      int nRow = 0;
      m_pcLuaMessageTable->clearContents();
      m_pcLuaMessageTable->setRowCount(m_vecControllers.size());
      for(size_t i = 0; i < m_vecControllers.size(); ++i) {
         if(! m_vecControllers[i]->IsOK()) {
            vecFields.clear();
            Tokenize(m_vecControllers[i]->GetErrorMessage(), vecFields, ":");
            m_pcLuaMessageTable->setItem(
               nRow, 0,
               new QTableWidgetItem(
                  QString::fromStdString(m_vecControllers[i]->GetId())));
            m_pcLuaMessageTable->setItem(
               nRow, 1,
               new QTableWidgetItem(
                  QString::fromStdString(vecFields[1])));
            m_pcLuaMessageTable->setItem(
               nRow, 2,
               new QTableWidgetItem(
                  QString::fromStdString(vecFields[2])));
            ++nRow;
         }
      }
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::HandleMsgTableSelection() {
      QList<QTableWidgetItem*> listSel = m_pcLuaMessageTable->selectedItems();
      if(! listSel.empty()) {
         int nLine = listSel[1]->data(Qt::DisplayRole).toInt();
         QTextCursor cCursor = m_pcCodeEditor->textCursor();
         int nCurLine = cCursor.blockNumber();
         if(nCurLine < nLine) {
            cCursor.movePosition(QTextCursor::NextBlock,
                                 QTextCursor::MoveAnchor,
                                 nLine - nCurLine - 1);
         }
         else if(nCurLine > nLine) {
            cCursor.movePosition(QTextCursor::PreviousBlock,
                                 QTextCursor::MoveAnchor,
                                 nCurLine - nLine + 1);
         }
         cCursor.movePosition(QTextCursor::StartOfBlock);
         m_pcCodeEditor->setTextCursor(cCursor);
         m_pcCodeEditor->setFocus();
      }
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::UpdateRecentFiles() {
      QSettings cSettings;
      cSettings.beginGroup("LuaEditor");
      QStringList listFiles = cSettings.value("recent_files").toStringList();
      int nRecentFiles = qMin(listFiles.size(), (int)MAX_RECENT_FILES);
      for(int i = 0; i < nRecentFiles; ++i) {
         m_pcFileOpenRecentAction[i]->setText(tr("&%1 %2").arg(i+1).arg(StrippedFileName(listFiles[i])));
         m_pcFileOpenRecentAction[i]->setData(listFiles[i]);
         m_pcFileOpenRecentAction[i]->setVisible(true);
      }
      for(int i = nRecentFiles; i < MAX_RECENT_FILES; ++i) {
         m_pcFileOpenRecentAction[i]->setVisible(false);
      }
      m_pcFileSeparateRecentAction->setVisible(nRecentFiles > 0);
      cSettings.endGroup();
   }

   /****************************************/
   /****************************************/

   QString CQTOpenGLLuaMainWindow::StrippedFileName(const QString& str_path) {
      return QFileInfo(str_path).fileName();
   }

   /****************************************/
   /****************************************/

   void CQTOpenGLLuaMainWindow::closeEvent(QCloseEvent* pc_event) {
      pc_event->ignore();
   }

   /****************************************/
   /****************************************/

}