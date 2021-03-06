/***************************************************************************
 * hörbert Software
 * Copyright (C) 2019 WINZKI GmbH & Co. KG
 *
 * Authors of the original version: Igor Yalovenko, Rainer Brang
 * Dec. 2019
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include "playlistpage.h"

#include <exception>

#include <QLabel>
#include <QLayout>
#include <QComboBox>
#include <QListView>
#include <QLineEdit>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>
#include <QApplication>
#include <QStandardPaths>
#include <QKeyEvent>
#include <QDebug>
#include <QSettings>
#include <QAction>
#include <QMenu>

#include "piebutton.h"
#include "playlistview.h"
#include "audioinfothread.h"
#include "functions.h"

PlaylistPage::PlaylistPage(QWidget *parent)
    : QWidget(parent)
{

    m_dir = "";
    m_dirNum = 0;

    m_pal = palette();

    // set black background
    setAutoFillBackground(true);
    setPalette(m_pal);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(30, 10, 30, 30);
    m_mainLayout->setSpacing(10);

    m_toolLayout = new QHBoxLayout();
    m_toolLayout->setAlignment(Qt::AlignCenter);
    m_toolLayout->setMargin(0);
    m_toolLayout->setSpacing(5);

    m_leftToolLayout = new QHBoxLayout();
    m_leftToolLayout->setAlignment(Qt::AlignLeft);

    m_colorBlindHintImg = new QLabel(this);
    m_colorBlindHintImg->setObjectName("BlindHint");
    m_colorBlindHintImg->setStyleSheet("#BlindHint { background-color: white }");
    m_colorBlindHintImg->setScaledContents(true);
    m_colorBlindHintImg->setFixedSize(48, 48);

    m_silenceLabel = new QLabel(this);
    m_silenceLabel->setFont(QFont("Monospace", 10, QFont::DemiBold));
    m_silenceLabel->setText(tr("Silence:"));
    m_silenceLabel->setStyleSheet("QLabel{ color:#ffffff; }");

    m_silenceDuration = new QComboBox(this);
    m_silenceDuration->setFixedWidth(100);
    m_silenceDuration->setEditable(true);
    m_silenceDuration->lineEdit()->setReadOnly(true);
    m_silenceDuration->lineEdit()->setAlignment(Qt::AlignCenter);
    m_silenceDuration->addItems(QString("1s,2s,5s,10s,1min,2min").split(","));

    m_addSilenceButton = new PieButton(this);
    m_addSilenceButton->setFixedSize(32, 32);
    m_addSilenceButton->setOverlayPixmap(QPixmap(":/images/pie_overlay.png"));
    m_addSilenceButton->setMainPixmap(QPixmap(":/images/plus.png"));
    m_addSilenceButton->setShadowEnabled(false);
    m_addSilenceButton->setShortcut(QKeySequence("Ctrl+S"));
    m_addSilenceButton->setToolTip(tr("Add silence of given duration") + QString(" (Ctrl+S)"));

    m_leftToolLayout->addWidget(m_colorBlindHintImg);
    m_leftToolLayout->addSpacing(10);
    m_leftToolLayout->addWidget(m_silenceLabel);
    m_leftToolLayout->addWidget(m_silenceDuration);
    m_leftToolLayout->addWidget(m_addSilenceButton);

    m_centerToolLayout = new QHBoxLayout();
    m_centerToolLayout->setAlignment(Qt::AlignCenter);

    m_addButton = new PieButton(this);
    m_addButton->setFixedSize(32, 32);
    m_addButton->setOverlayPixmap(QPixmap(":/images/pie_overlay.png"));
    m_addButton->setMainPixmap(QPixmap(":/images/plus.png"));
    m_addButton->setShadowEnabled(false);
    m_addButton->setShortcut(QKeySequence("Ctrl+F"));
    m_addButton->setToolTip(tr("Add audio files or tracks from Finder/Explorer") + QString(" (Ctrl+F)"));

    m_removeButton = new PieButton(this);
    m_removeButton->setFixedSize(32, 32);
    m_removeButton->setBackgroundColor(QColor(255, 55, 55));
    m_removeButton->setOverlayPixmap(QPixmap(":/images/pie_overlay.png"));
    m_removeButton->setMainPixmap(QPixmap(":/images/minus.png"));
    m_removeButton->setShadowEnabled(false);
    m_removeButton->setShortcut(QKeySequence("Ctrl+R"));
    m_removeButton->setToolTip(tr("Remove tracks from the list") + QString(" (Ctrl+R)"));

    m_centerToolLayout->addWidget(m_addButton);
    m_centerToolLayout->addWidget(m_removeButton);

    m_rightToolLayout = new QHBoxLayout();
    m_rightToolLayout->setAlignment(Qt::AlignRight);

    m_commitButton = new PieButton(this);
    m_commitButton->setFixedSize(32, 32);
    m_commitButton->setOverlayPixmap(QPixmap(":/images/pie_overlay.png"));
    m_commitButton->setMainPixmap(QPixmap(":/images/confirm.png"));
    m_commitButton->setShadowEnabled(false);
    m_commitButton->setShortcut(QKeySequence("Ctrl+C"));
    m_commitButton->setToolTip(tr("Write changes to the card") + QString(" (Ctrl+C)"));

    m_cancelButton = new PieButton(this);
    m_cancelButton->setFixedSize(32, 32);
    m_cancelButton->setBackgroundColor(QColor(255, 55, 55));
    m_cancelButton->setOverlayPixmap(QPixmap(":/images/pie_overlay.png"));
    m_cancelButton->setMainPixmap(QPixmap(":/images/cancel.png"));
    m_cancelButton->setShadowEnabled(false);
    m_cancelButton->setShortcut(QKeySequence("Escape"));
    m_cancelButton->setToolTip(tr("Discard changes and exit playlist") + QString(" (Escape)"));

    m_rightToolLayout->addWidget(m_commitButton);
    m_rightToolLayout->addWidget(m_cancelButton);

    m_toolLayout->addLayout(m_leftToolLayout, 1);
    m_toolLayout->addLayout(m_centerToolLayout, 1);
    m_toolLayout->addLayout(m_rightToolLayout, 1);

    m_playlistView = new PlaylistView(this);

    m_mainLayout->addLayout(m_toolLayout);
    m_mainLayout->addWidget(m_playlistView);

    setTabOrder(m_playlistView, m_silenceDuration);
    m_playlistView->setFocus();

    m_contextMenu = new QMenu(this);

    m_actionAlbumVisible = new QAction(tr("Show album"), this);
    m_actionAlbumVisible->setCheckable(true);
    m_actionAlbumVisible->setChecked(columnVisibleFromSettings("album"));
    connect(m_actionAlbumVisible, &QAction::triggered, this, [this]() {
       this->updateColumnVisibleSettings("album", m_actionAlbumVisible->isChecked());
       this->m_playlistView->setColumnVisible(METADATA_ALBUM_COLUMN_INDEX, m_actionAlbumVisible->isChecked());
    });

    m_actionCommentVisible = new QAction(tr("Show paths"), this);
    m_actionCommentVisible->setCheckable(true);
    m_actionCommentVisible->setChecked(columnVisibleFromSettings("comment"));
    connect(m_actionCommentVisible, &QAction::triggered, this, [this]() {
       this->updateColumnVisibleSettings("comment", m_actionCommentVisible->isChecked());
       this->m_playlistView->setColumnVisible(METADATA_COMMENT_COLUMN_INDEX, m_actionCommentVisible->isChecked());
    });

    m_contextMenu->setTitle(tr("Show/Hide columns"));
    m_contextMenu->addSeparator();

    m_contextMenu->addAction(m_actionAlbumVisible);
    m_contextMenu->addAction(m_actionCommentVisible);

    if (!columnVisibleFromSettings("album"))
        m_playlistView->setColumnVisible(METADATA_ALBUM_COLUMN_INDEX, false);

    if (!columnVisibleFromSettings("comment"))
        m_playlistView->setColumnVisible(METADATA_COMMENT_COLUMN_INDEX, false);

    connect(m_addSilenceButton, &QPushButton::clicked, this, &PlaylistPage::addSilence);
    connect(m_addButton, &QPushButton::clicked, this, &PlaylistPage::add);
    connect(m_removeButton, &QPushButton::clicked, this, &PlaylistPage::remove);
    connect(m_commitButton, &QPushButton::clicked, this, [this] () {
        emit onClosePage(true);
    });
    connect(m_cancelButton, &QPushButton::clicked, this, [this] () {
        emit onClosePage(false);
    });
    connect(m_playlistView, &PlaylistView::durationChanged, this, &PlaylistPage::durationChanged);

    connect(m_playlistView, &PlaylistView::errorOccurred, this, &PlaylistPage::errorOccurred);
}

void PlaylistPage::setDirecytory(const QString &dir_path, quint8 dir_num, const AudioList &result)
{
    m_dir = dir_path;
    m_dirNum = dir_num;
    assert(m_dirNum < 9);

    m_colorBlindHintImg->setPixmap(QPixmap(QString(":/images/colorblind_hint_0%1.png").arg(dir_num + 1)).copy(100, 100, 400, 400));

    m_originalList[dir_num] = result;
    m_playlistView->load(result, dir_num);

    AudioList initial_list = m_playlistView->getEntryList();

    AudioList implicit_list;
    for (const auto& key : initial_list.keys())
    {
        // we don't need to put the added entries in count while processing moved entries
        if (m_originalList[dir_num].keys().contains(key))
        {
            // check whether it is moved or not.
            auto file_name = getFileNameWithoutExtension(initial_list[key].path);
            if (file_name != initial_list[key].order)
            {
                implicit_list.insert(key, initial_list.value(key));
            }
        }
    }

    m_implicitlyMovedList[dir_num] = implicit_list;
}

quint8 PlaylistPage::directory()
{
    return m_dirNum;
}

QString PlaylistPage::directoryPath()
{
    return m_dir;
}

void PlaylistPage::setBackgroundColor(const QColor &color)
{
    m_pal.setColor(QPalette::Window, color);
    setPalette(m_pal);

    m_playlistView->setDropIndicatorColor(color);
}

void PlaylistPage::moveSelectedEntriesTo(quint8 toDirNum, bool add2Beginning)
{
    m_clipBoard = m_playlistView->getSelectedEntries();
    if (m_clipBoard.count() == 0)
    {
        QMessageBox::information(this, tr("Move"), tr("Please select the files you want to move."));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString source_dir = QString();
#ifdef _WIN32
    source_dir = m_dir + QString::number(m_dirNum) + "/";
#else
    source_dir = m_dir + "/" + QString::number(m_dirNum) + "/";
#endif

    QString dest_dir = QString();
#ifdef _WIN32
    dest_dir = m_dir + QString::number(toDirNum) + "/";
#else
    dest_dir = m_dir + "/" + QString::number(toDirNum) + "/";
#endif
    assert(!source_dir.isEmpty() && !dest_dir.isEmpty());

    // if the choice is "beginning of", we need to rename files in destination directory by offset
    auto file_count = getFileCount(dest_dir);
    if (add2Beginning && m_clipBoard.count() > 0 && file_count > 0)
    {
        /* sometimes, first file does not start from 0
           in that case, we need to get the offset by subtracting first number in the directory */
        auto first_number = getFirstNumberInDirectory(dest_dir);
        auto offset = m_clipBoard.count() - first_number;

        if (offset > 0)
        {
            auto renamed_count = batchRenameByIndex(dest_dir, 0, offset);
            if (renamed_count != file_count)
            {
                qCritical() << "Failed to rename original files" << renamed_count << "/" << file_count;
                return;
            }
        }
    }

    // read the destination directory and get last number in names
    auto max_number = getLastNumberInDirectory(dest_dir);

    max_number++;

    // for choice "beginning" start from 0 but for choice "end of", start from the last number + 1
    auto index = add2Beginning ? 0 : max_number;

    std::list<int> row_index_list;

    // then we move the selected files to desired directory
    for (const auto& entry : m_clipBoard)
    {
        QString new_file_name = dest_dir + QString::number(index++) + DEFAULT_DESTINATION_FORMAT;

        if (moveFile(entry.path, new_file_name))
        {
            row_index_list.push_back(entry.order);
        }
    }

    row_index_list.sort(std::greater<int>());

    for (const auto& index : row_index_list)
    {
        m_playlistView->removeRow(index);
    }
    QApplication::restoreOverrideCursor();
}

void PlaylistPage::clearDirectoryEstimation(quint8 dirIndex)
{
    m_playlistView->clearDirectoryEstimation(dirIndex);
}

void PlaylistPage::addSilence()
{
    // do some stuffs for actual commands to generate silence wav files
    m_playlistView->addSilence(getSelectedSilenceDurationInSeconds());
}

void PlaylistPage::add()
{
    QFileDialog dlg(this);
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.setNameFilter(QString(tr("Audio Files (%1)")).arg(SUPPORTED_FILES));
    dlg.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dlg.exec() == QDialog::Accepted)
        fileNames = dlg.selectedFiles();
    else
        return;

    QFileInfoList file_info_list;

    for (const auto& file : fileNames)
    {
        file_info_list.append(QFileInfo(file));
    }

    QCoreApplication::processEvents();

    m_playlistView->readEntries(file_info_list, m_playlistView->currentRow() + 1);

    //std::sort(file_info_list.begin(), file_info_list.end(), sortByNumber);
}

void PlaylistPage::remove()
{
    if (m_playlistView->selectionModel()->selectedRows().count() <= 0)
    {
        QMessageBox::information(this, tr("Remove"), tr("Please select the files you want to remove."));
        return;
    }
    m_playlistView->remove();
}

void PlaylistPage::discard()
{
    m_playlistView->stopPlayer();
    emit cancelClicked();
}

void PlaylistPage::setDriveSpaceDetails(quint64 used, quint64 total, quint64 estimatedSeconds)
{
    m_playlistView->setDriveSpaceDetails(used, total, estimatedSeconds);
}

void PlaylistPage::contextMenuEvent(QContextMenuEvent *e)
{
    m_contextMenu->exec(e->globalPos());
}

void PlaylistPage::onClosePage(bool commitChanges)
{
    m_playlistView->stopPlayer();
    auto original_list = m_originalList[m_dirNum];
    auto changed_list = m_playlistView->getEntryList();

    auto original_key_list = m_originalList[m_dirNum].keys();
    auto changed_key_list = changed_list.keys();

    // entries that were on the selected card and later removed by user
    AudioList removed_entries;

    for (const auto& key : original_key_list)
    {
        if (!changed_key_list.contains(key))
            removed_entries.insert(key, original_list.value(key));
    }

    // entries added by user (added by + button, drag & drop or silence)
    AudioList added_entries;

    for (const auto& key : changed_key_list)
    {
        // do not include split files
        if ( !original_key_list.contains(key) && (changed_list.value(key).state == 0) )
            added_entries.insert(key, changed_list.value(key));
    }

    auto added_key_list = added_entries.keys();

    // entries moved by user
    AudioList moved_entries;

    for (const auto& key : changed_key_list)
    {
        // we don't need to put the added entries in count while processing moved entries
        if (original_key_list.contains(key))
        {
            // check whether it is moved or not.
            auto file_name = getFileNameWithoutExtension(changed_list[key].path);
            if (file_name != changed_list[key].order)
            {
                moved_entries.insert(key, changed_list.value(key));
            }
        }
    }

    // entries split into pieces
    AudioList split_entries;

    for (const auto& entry : changed_list)
    {
        if (entry.state != 0)
            split_entries.insert(entry.id, entry);
    }

    // entries whose metadata have been changed, ensure not to include added entries
    AudioList metadata_entries;

    for (const auto& entry : changed_list)
    {
        if (original_key_list.contains(entry.id) && entry.metadata != original_list[entry.id].metadata)
        {
            qDebug() << entry.metadata.title << original_list[entry.id].metadata.title;
            qDebug() << entry.metadata.album << original_list[entry.id].metadata.album;
            qDebug() << entry.metadata.comment << original_list[entry.id].metadata.comment;
            metadata_entries.insert(entry.id, entry);
        }
    }

    bool has_implicit_changes =  (m_implicitlyMovedList[m_dirNum].count() == moved_entries.count());
    bool has_explicit_changes = (removed_entries.count() > 0 || removed_entries.count() > 0
                                 || split_entries.count() > 0 || metadata_entries.count() > 0);

    /**
     * sometimes the app has automatic implicit changes, need to deal with them on cancel click
     * i.e. 0.WAV, 1.WAV, 3.WAV will automatically be ->> 0.WAV, 1.WAV, 2.WAV
     * */
    if (!commitChanges)
    {
        if (has_explicit_changes || !has_implicit_changes)
        {
            auto selected = QMessageBox::question(this, tr("Playlist"), tr("You have unsaved changes.\n Do you want to cancel all changes?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::No );

            if (selected == QMessageBox::Yes)
            {
                emit cancelClicked();
            }

            return;
        }
        else {
            emit cancelClicked();
            return;
        }
    }

    QMap<ENTRY_LIST_TYPE, AudioList> commit_list;

    if (removed_entries.count() > 0)
        commit_list.insert(REMOVED_ENTRIES, removed_entries);

    if (added_entries.count() > 0)
        commit_list.insert(ADDED_ENTRIES, added_entries);

    if (moved_entries.count() > 0)
        commit_list.insert(MOVED_ENTRIES, moved_entries);

    if (split_entries.count() > 0)
        commit_list.insert(SPLIT_ENTRIES, split_entries);

    if (metadata_entries.count() > 0)
        commit_list.insert(METADATA_CHANGED_ENTRIES, metadata_entries);

    emit this->commitChanges(commit_list);
}

int PlaylistPage::getSelectedSilenceDurationInSeconds()
{
    int duration = -1;
    QString value = m_silenceDuration->currentText();
    if (value.endsWith("s"))
    {
        duration = value.section("s", 0, 0).toInt();
    }
    else if (value.endsWith("min"))
    {
        duration = value.section("min", 0, 0).toInt() * 60;
    }
    return duration;
}

void PlaylistPage::updateColumnVisibleSettings(const QString &columnName, bool visible)
{
    QSettings settings;
    settings.beginGroup("Playlist");
    settings.setValue(columnName, visible);
    settings.endGroup();
}

bool PlaylistPage::columnVisibleFromSettings(const QString &columnName)
{
    bool ret;
    QSettings settings;
    settings.beginGroup("Playlist");
    ret = settings.value(columnName).toBool();
    settings.endGroup();

    return ret;
}



