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

#ifndef ADVANCEDFEATURESDIALOG_H
#define ADVANCEDFEATURESDIALOG_H

#include <QDialog>

class QSettings;
class QLabel;
class QRadioButton;
class QObject;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QPushButton;
class QCheckBox;

/**
 * @brief The AdvancedFeaturesDialog Class
 *
 * Advanced features to be settable for "pro" users:
 * Advanced features may have to be set by pro users or when a user is in a service call,
 * so the service agent can ask the user to specifically set something here.
 */
class AdvancedFeaturesDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief AdvancedFeaturesDialog
     * @param parent
     */
    AdvancedFeaturesDialog(QWidget *parent = Q_NULLPTR);

    /**
     * @brief readSettings
     */
    void readSettings();

    /**
     * @brief writeVolumeSettings
     */
    void writeVolumeSettings(const QString &);

    /**
     * @brief enable/disable diagnostics switching
     * @param enable
     */
    void setDiagnosticsSwitchingEnabled(bool enable);

    /**
     * @brief collect information for support and output to a plain text file
     */
    void collectInformationForSupport();

signals:
    /**
     * @brief this signal is emitted when state of diagnostics checkbox is changed
     * @param enabled true when the mode is enabled, otherwise false
     */
    void diagnosticsModeSwitched(bool enabled);

    /**
     * @brief this signal is emitted when user click on collect button to get information for support
     */
    void collectInformationForSupportRequested();

private:
    QSettings *m_settings;

    QLabel *m_versionLabel;
    QLabel *m_companyLabel;
    QLabel *m_siteUrlLabel;
    QLabel *m_maxVolumeLabel;

    QRadioButton *m_lowVolumeOption;
    QRadioButton *m_normalVolumeOption;
    QRadioButton *m_maxVolumeOption;

    QCheckBox *m_reminderOption;
    QCheckBox *m_showLargeDriveCheck;

    QLabel *m_diagnosticsLabel;
    QPushButton *m_diagnosticsButton;

    QPushButton *m_collectSupportInfoButton;

    QPushButton *m_closeButton;

    QVBoxLayout *m_layout;
    QHBoxLayout *m_companyLayout;
    QHBoxLayout *m_optionLayout;
    QVBoxLayout *m_checkLayout;
    QHBoxLayout *m_diagLayout;
};

#endif // ADVANCEDFEATURESDIALOG_H
