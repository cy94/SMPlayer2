/*  smplayer2, GUI front-end for mplayer2.
    Copyright (C) 2006-2010 Ricardo Villalba <rvm@escomposlinux.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "prefgeneral.h"
#include "preferences.h"
#include "filedialog.h"
#include "images.h"
#include "mediasettings.h"
#include "paths.h"
#include "vdpauproperties.h"

#if USE_ALSA_DEVICES || USE_DSOUND_DEVICES
#include "deviceinfo.h"
#endif

PrefGeneral::PrefGeneral(QWidget *parent, Qt::WindowFlags f)
    : PrefWidget(parent, f)
{
    setupUi(this);

    mplayerbin_edit->setDialogType(FileChooser::GetFileName);
    screenshot_edit->setDialogType(FileChooser::GetDirectory);

    // Read driver info from InfoReader:
    InfoReader *i = InfoReader::obj();
    vo_list = i->voList();
    ao_list = i->aoList();

#if USE_DSOUND_DEVICES
    dsound_devices = DeviceInfo::dsoundDevices();
#endif

#if USE_ALSA_DEVICES
    alsa_devices = DeviceInfo::alsaDevices();
#endif
#if USE_XV_ADAPTORS
    xv_adaptors = DeviceInfo::xvAdaptors();
#endif

    // Screensaver
#ifdef Q_OS_WIN
    screensaver_check->hide();
#else
    screensaver_group->hide();
#endif

#ifdef Q_OS_WIN
    vdpau_button->hide();
#endif

    // Channels combo
    channels_combo->addItem("2", MediaSettings::ChStereo);
    channels_combo->addItem("4", MediaSettings::ChQuad);
    channels_combo->addItem("6", MediaSettings::Ch51);
    channels_combo->addItem("8", MediaSettings::Ch71);

    connect(vo_combo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(vo_combo_changed(int)));
    connect(ao_combo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(ao_combo_changed(int)));

    retranslateStrings();
}

PrefGeneral::~PrefGeneral()
{
}

QString PrefGeneral::sectionName()
{
    return tr("General");
}

QPixmap PrefGeneral::sectionIcon()
{
    return Images::icon("pref_general");
}

void PrefGeneral::retranslateStrings()
{
    retranslateUi(this);

    channels_combo->setItemText(0, tr("2 (Stereo)"));
    channels_combo->setItemText(1, tr("4 (4.0 Surround)"));
    channels_combo->setItemText(2, tr("6 (5.1 Surround)"));
    channels_combo->setItemText(3, tr("8 (7.1 Surround)"));

    int deinterlace_item = deinterlace_combo->currentIndex();
    deinterlace_combo->clear();
    deinterlace_combo->addItem(tr("None"), MediaSettings::NoDeinterlace);
    deinterlace_combo->addItem(tr("Lowpass5"), MediaSettings::L5);
    deinterlace_combo->addItem(tr("Yadif (normal)"), MediaSettings::Yadif);
    deinterlace_combo->addItem(tr("Yadif (double framerate)"), MediaSettings::Yadif_1);
    deinterlace_combo->addItem(tr("Linear Blend"), MediaSettings::LB);
    deinterlace_combo->addItem(tr("Kerndeint"), MediaSettings::Kerndeint);
    deinterlace_combo->setCurrentIndex(deinterlace_item);

    int filesettings_method_item = filesettings_method_combo->currentIndex();
    filesettings_method_combo->clear();
    filesettings_method_combo->addItem(tr("one ini file"), "normal");
    filesettings_method_combo->addItem(tr("multiple ini files"), "hash");
    filesettings_method_combo->setCurrentIndex(filesettings_method_item);

    updateDriverCombos();

    // Icons
    /*
    resize_window_icon->setPixmap( Images::icon("resize_window") );
    volume_icon->setPixmap( Images::icon("speaker") );
    */

    mplayerbin_edit->setCaption(tr("Select the mplayer executable"));
#ifdef Q_OS_WIN
    mplayerbin_edit->setFilter(tr("Executables") + " (*.exe)");
#else
    mplayerbin_edit->setFilter(tr("All files") + " (*)");
#endif
    screenshot_edit->setCaption(tr("Select a directory"));

    preferred_desc->setText(
        tr("Here you can type your preferred language for the audio "
           "and subtitle streams. When a media with multiple audio or "
           "subtitle streams is found, SMPlayer2 will try to use your "
           "preferred language. This only will work with media that offer "
           "info about the language of audio and subtitle streams, like DVDs "
           "or mkv files.<br>These fields accept regular expressions. "
           "Example: <b>es|esp|spa</b> will select the track if it matches with "
           "<i>es</i>, <i>esp</i> or <i>spa</i>."));

    createHelp();
}

void PrefGeneral::setData(Preferences *pref)
{
    setMplayerPath(pref->mplayer_bin);

    setScreenshotDir(pref->screenshot_directory);

    QString vo = pref->vo;

    if (vo.isEmpty()) {
#ifdef Q_OS_WIN
        vo = "direct3d,";
#else
        vo = "xv,";
#endif
    }

    setVO(vo);

    QString ao = pref->ao;
    setAO(ao);

    setRememberSettings(!pref->dont_remember_media_settings);
    setRememberTimePos(!pref->dont_remember_time_pos);
    setFileSettingsMethod(pref->file_settings_method);
    setAudioLang(pref->audio_lang);
    setSubtitleLang(pref->subtitle_lang);
    setAudioTrack(pref->initial_audio_track);
    setSubtitleTrack(pref->initial_subtitle_track);
    setCloseOnFinish(pref->close_on_finish);
    setPauseWhenHidden(pref->pause_when_hidden);

    setEq2(pref->use_soft_video_eq);
    setUseAudioEqualizer(pref->use_audio_equalizer);
    setGlobalVolume(pref->global_volume);
    setSoftVol(pref->use_soft_vol);
    setAc3DTSPassthrough(pref->use_hwac3);
    setGaplessAudio(pref->gapless_audio);
    setInitialVolNorm(pref->initial_volnorm);
    setAmplification(pref->softvol_max);
    setInitialDeinterlace(pref->initial_deinterlace);
    setInitialZoom(pref->initial_zoom_factor);
    setStartInFullscreen(pref->start_in_fullscreen);
    setBlackbordersOnFullscreen(pref->add_blackborders_on_fullscreen);

#ifdef Q_OS_WIN
    setAvoidScreensaver(pref->avoid_screensaver);
    setTurnScreensaverOff(pref->turn_screensaver_off);
#else
    setDisableScreensaver(pref->disable_screensaver);
#endif

#ifndef Q_OS_WIN
    vdpau = pref->vdpau;
#endif

    setAudioChannels(pref->initial_audio_channels);
    setScaleTempoFilter(pref->use_scaletempo);

    setAutoSyncActivated(pref->autosync);
    setAutoSyncFactor(pref->autosync_factor);

    setMcActivated(pref->use_mc);
    setMc(pref->mc_value);
}

void PrefGeneral::getData(Preferences *pref)
{
    requires_restart = false;
    filesettings_method_changed = false;

    if (pref->mplayer_bin != mplayerPath()) {
        requires_restart = true;
        pref->mplayer_bin = mplayerPath();

        qDebug("PrefGeneral::getData: mplayer binary has changed, getting version number");
        // Forces to get info from mplayer to update version number
        InfoReader i(pref->mplayer_bin);
        i.getInfo();
        // Update the drivers list at the same time
        //setDrivers( i.voList(), i.aoList() );
    }

    TEST_AND_SET(pref->screenshot_directory, screenshotDir());
    TEST_AND_SET(pref->vo, VO());
    TEST_AND_SET(pref->ao, AO());

    bool dont_remember_ms = !rememberSettings();
    TEST_AND_SET(pref->dont_remember_media_settings, dont_remember_ms);

    bool dont_remember_time = !rememberTimePos();
    TEST_AND_SET(pref->dont_remember_time_pos, dont_remember_time);

    if (pref->file_settings_method != fileSettingsMethod()) {
        pref->file_settings_method = fileSettingsMethod();
        filesettings_method_changed = true;
    }

    pref->audio_lang = audioLang();
    pref->subtitle_lang = subtitleLang();

    pref->initial_audio_track = audioTrack();
    pref->initial_subtitle_track = subtitleTrack();

    pref->close_on_finish = closeOnFinish();
    pref->pause_when_hidden = pauseWhenHidden();

    TEST_AND_SET(pref->use_soft_video_eq, eq2());
    TEST_AND_SET(pref->use_soft_vol, softVol());
    pref->global_volume = globalVolume();
    TEST_AND_SET(pref->use_audio_equalizer, useAudioEqualizer());
    TEST_AND_SET(pref->use_hwac3, Ac3DTSPassthrough());
    TEST_AND_SET(pref->gapless_audio, GaplessAudio());
    pref->initial_volnorm = initialVolNorm();
    TEST_AND_SET(pref->softvol_max, amplification());
    pref->initial_deinterlace = initialDeinterlace();
    pref->initial_zoom_factor = initialZoom();
    pref->start_in_fullscreen = startInFullscreen();

    if (pref->add_blackborders_on_fullscreen != blackbordersOnFullscreen()) {
        pref->add_blackborders_on_fullscreen = blackbordersOnFullscreen();

        if (pref->fullscreen) requires_restart = true;
    }

#ifdef Q_OS_WIN
    pref->avoid_screensaver = avoidScreensaver();
    TEST_AND_SET(pref->turn_screensaver_off, turnScreensaverOff());
#else
    TEST_AND_SET(pref->disable_screensaver, disableScreensaver());
#endif

#ifndef Q_OS_WIN
    pref->vdpau = vdpau;
#endif

    pref->initial_audio_channels = audioChannels();
    TEST_AND_SET(pref->use_scaletempo, scaleTempoFilter());

    TEST_AND_SET(pref->autosync, autoSyncActivated());
    TEST_AND_SET(pref->autosync_factor, autoSyncFactor());

    TEST_AND_SET(pref->use_mc, mcActivated());
    TEST_AND_SET(pref->mc_value, mc());
}

void PrefGeneral::updateDriverCombos()
{
    int vo_current = vo_combo->currentIndex();
    int ao_current = ao_combo->currentIndex();

    vo_combo->clear();
    ao_combo->clear();

    QString vo;

    for (int n = 0; n < vo_list.count(); n++) {
        vo = vo_list[n].name();
#ifdef Q_OS_WIN

        if (vo == "directx") {
            vo_combo->addItem("directx (" + tr("fast") + ")", "directx");
            vo_combo->addItem("directx (" + tr("slow") + ")", "directx:noaccel");
        } else
#else
        /*
        if (vo == "xv") vo_combo->addItem( "xv (" + tr("fastest") + ")", vo);
        else
        */
#if USE_XV_ADAPTORS
        if ((vo == "xv") && (!xv_adaptors.isEmpty())) {
            vo_combo->addItem(vo, vo);

            for (int n = 0; n < xv_adaptors.count(); n++) {
                vo_combo->addItem("xv (" + xv_adaptors[n].ID().toString() + " - " + xv_adaptors[n].desc() + ")",
                                  "xv:adaptor=" + xv_adaptors[n].ID().toString());
            }
        } else
#endif // USE_XV_ADAPTORS
#endif
            if (vo == "x11") vo_combo->addItem("x11 (" + tr("slow") + ")", vo);
            else if (vo == "gl") {
                vo_combo->addItem(vo, vo);
                vo_combo->addItem("gl (" + tr("fast") + ")", "gl:yuv=2:force-pbo");
                vo_combo->addItem("gl (" + tr("fast - ATI cards") + ")", "gl:yuv=2:force-pbo:ati-hack");
                vo_combo->addItem("gl (yuv)", "gl:yuv=3");
            } else if (vo == "gl2") {
                vo_combo->addItem(vo, vo);
                vo_combo->addItem("gl2 (yuv)", "gl2:yuv=3");
            } else if (vo == "null" || vo == "png" || vo == "jpeg" || vo == "gif89a" ||
                       vo == "tga" || vo == "pnm" || vo == "md5sum") {
                ; // Nothing to do
            } else
                vo_combo->addItem(vo, vo);
    }

    vo_combo->addItem(tr("User defined..."), "user_defined");

    QString ao;

    for (int n = 0; n < ao_list.count(); n++) {
        ao = ao_list[n].name();
        ao_combo->addItem(ao, ao);
#if USE_ALSA_DEVICES

        if ((ao == "alsa") && (!alsa_devices.isEmpty())) {
            for (int n = 0; n < alsa_devices.count(); n++) {
                ao_combo->addItem("alsa (" + alsa_devices[n].ID().toString() + " - " + alsa_devices[n].desc() + ")",
                                  "alsa:device=hw=" + alsa_devices[n].ID().toString());
            }
        }

#endif
#if USE_DSOUND_DEVICES

        if ((ao == "dsound") && (!dsound_devices.isEmpty())) {
            for (int n = 0; n < dsound_devices.count(); n++) {
                ao_combo->addItem("dsound (" + dsound_devices[n].ID().toString() + " - " + dsound_devices[n].desc() + ")",
                                  "dsound:device=" + dsound_devices[n].ID().toString());
            }
        }

#endif
    }

    ao_combo->addItem(tr("User defined..."), "user_defined");

    vo_combo->setCurrentIndex(vo_current);
    ao_combo->setCurrentIndex(ao_current);
}

void PrefGeneral::setMplayerPath(QString path)
{
    mplayerbin_edit->setText(path);
}

QString PrefGeneral::mplayerPath()
{
    return mplayerbin_edit->text();
}

void PrefGeneral::setScreenshotDir(QString path)
{
    screenshot_edit->setText(path);
}

QString PrefGeneral::screenshotDir()
{
    return screenshot_edit->text();
}

void PrefGeneral::setVO(QString vo_driver)
{
    int idx = vo_combo->findData(vo_driver);

    if (idx != -1) {
        vo_combo->setCurrentIndex(idx);
    } else {
        vo_combo->setCurrentIndex(vo_combo->findData("user_defined"));
        vo_user_defined_edit->setText(vo_driver);
    }

    vo_combo_changed(vo_combo->currentIndex());
}

void PrefGeneral::setAO(QString ao_driver)
{
    int idx = ao_combo->findData(ao_driver);

    if (idx != -1) {
        ao_combo->setCurrentIndex(idx);
    } else {
        ao_combo->setCurrentIndex(ao_combo->findData("user_defined"));
        ao_user_defined_edit->setText(ao_driver);
    }

    ao_combo_changed(ao_combo->currentIndex());
}

QString PrefGeneral::VO()
{
    QString vo = vo_combo->itemData(vo_combo->currentIndex()).toString();

    if (vo == "user_defined") {
        vo = vo_user_defined_edit->text();
        /*
        if (vo.isEmpty()) {
        	vo = vo_combo->itemData(0).toString();
        	qDebug("PrefGeneral::VO: user defined vo is empty, using %s", vo.toUtf8().constData());
        }
        */
    }

    return vo;
}

QString PrefGeneral::AO()
{
    QString ao = ao_combo->itemData(ao_combo->currentIndex()).toString();

    if (ao == "user_defined") {
        ao = ao_user_defined_edit->text();
        /*
        if (ao.isEmpty()) {
        	ao = ao_combo->itemData(0).toString();
        	qDebug("PrefGeneral::AO: user defined ao is empty, using %s", ao.toUtf8().constData());
        }
        */
    }

    return ao;
}

void PrefGeneral::setRememberSettings(bool b)
{
    remember_all_check->setChecked(b);
    //rememberAllButtonToggled(b);
}

bool PrefGeneral::rememberSettings()
{
    return remember_all_check->isChecked();
}

void PrefGeneral::setRememberTimePos(bool b)
{
    remember_time_check->setChecked(b);
}

bool PrefGeneral::rememberTimePos()
{
    return remember_time_check->isChecked();
}

void PrefGeneral::setFileSettingsMethod(QString method)
{
    int index = filesettings_method_combo->findData(method);

    if (index < 0) index = 0;

    filesettings_method_combo->setCurrentIndex(index);
}

QString PrefGeneral::fileSettingsMethod()
{
    return filesettings_method_combo->itemData(filesettings_method_combo->currentIndex()).toString();
}

void PrefGeneral::setAudioLang(QString lang)
{
    audio_lang_edit->setText(lang);
}

QString PrefGeneral::audioLang()
{
    return audio_lang_edit->text();
}

void PrefGeneral::setSubtitleLang(QString lang)
{
    subtitle_lang_edit->setText(lang);
}

QString PrefGeneral::subtitleLang()
{
    return subtitle_lang_edit->text();
}

void PrefGeneral::setAudioTrack(int track)
{
    audio_track_spin->setValue(track);
}

int PrefGeneral::audioTrack()
{
    return audio_track_spin->value();
}

void PrefGeneral::setSubtitleTrack(int track)
{
    subtitle_track_spin->setValue(track);
}

int PrefGeneral::subtitleTrack()
{
    return subtitle_track_spin->value();
}

void PrefGeneral::setCloseOnFinish(bool b)
{
    close_on_finish_check->setChecked(b);
}

bool PrefGeneral::closeOnFinish()
{
    return close_on_finish_check->isChecked();
}

void PrefGeneral::setPauseWhenHidden(bool b)
{
    pause_if_hidden_check->setChecked(b);
}

bool PrefGeneral::pauseWhenHidden()
{
    return pause_if_hidden_check->isChecked();
}


void PrefGeneral::setEq2(bool b)
{
    eq2_check->setChecked(b);
}

bool PrefGeneral::eq2()
{
    return eq2_check->isChecked();
}

void PrefGeneral::setSoftVol(bool b)
{
    softvol_check->setChecked(b);
}

void PrefGeneral::setGlobalVolume(bool b)
{
    global_volume_check->setChecked(b);
}

bool PrefGeneral::globalVolume()
{
    return global_volume_check->isChecked();
}

bool PrefGeneral::softVol()
{
    return softvol_check->isChecked();
}

void PrefGeneral::setAutoSyncFactor(int factor)
{
    autosync_spin->setValue(factor);
}

int PrefGeneral::autoSyncFactor()
{
    return autosync_spin->value();
}

void PrefGeneral::setAutoSyncActivated(bool b)
{
    autosync_check->setChecked(b);
}

bool PrefGeneral::autoSyncActivated()
{
    return autosync_check->isChecked();
}

void PrefGeneral::setMc(double value)
{
    mc_spin->setValue(value);
}

double PrefGeneral::mc()
{
    return mc_spin->value();
}

void PrefGeneral::setMcActivated(bool b)
{
    use_mc_check->setChecked(b);
}

bool PrefGeneral::mcActivated()
{
    return use_mc_check->isChecked();
}

void PrefGeneral::setUseAudioEqualizer(bool b)
{
    audio_equalizer_check->setChecked(b);
}

bool PrefGeneral::useAudioEqualizer()
{
    return audio_equalizer_check->isChecked();
}

void PrefGeneral::setAc3DTSPassthrough(bool b)
{
    hwac3_check->setChecked(b);
}

bool PrefGeneral::Ac3DTSPassthrough()
{
    return hwac3_check->isChecked();
}

void PrefGeneral::setGaplessAudio(bool b)
{
    gapless_audio_check->setChecked(b);
}

bool PrefGeneral::GaplessAudio()
{
    return gapless_audio_check->isChecked();
}

void PrefGeneral::setInitialVolNorm(bool b)
{
    volnorm_check->setChecked(b);
}

bool PrefGeneral::initialVolNorm()
{
    return volnorm_check->isChecked();
}

void PrefGeneral::setInitialDeinterlace(int ID)
{
    int pos = deinterlace_combo->findData(ID);

    if (pos != -1) {
        deinterlace_combo->setCurrentIndex(pos);
    } else {
        qWarning("PrefGeneral::setInitialDeinterlace: ID: %d not found in combo", ID);
    }
}

int PrefGeneral::initialDeinterlace()
{
    if (deinterlace_combo->currentIndex() != -1) {
        return deinterlace_combo->itemData(deinterlace_combo->currentIndex()).toInt();
    } else {
        qWarning("PrefGeneral::initialDeinterlace: no item selected");
        return 0;
    }
}

void PrefGeneral::setInitialZoom(double v)
{
    zoom_spin->setValue(v);
}

double PrefGeneral::initialZoom()
{
    return zoom_spin->value();
}

void PrefGeneral::setAmplification(int n)
{
    softvol_max_spin->setValue(n);
}

int PrefGeneral::amplification()
{
    return softvol_max_spin->value();
}

void PrefGeneral::setAudioChannels(int ID)
{
    int pos = channels_combo->findData(ID);

    if (pos != -1) {
        channels_combo->setCurrentIndex(pos);
    } else {
        qWarning("PrefGeneral::setAudioChannels: ID: %d not found in combo", ID);
    }
}

int PrefGeneral::audioChannels()
{
    if (channels_combo->currentIndex() != -1) {
        return channels_combo->itemData(channels_combo->currentIndex()).toInt();
    } else {
        qWarning("PrefGeneral::audioChannels: no item selected");
        return 0;
    }
}

void PrefGeneral::setStartInFullscreen(bool b)
{
    start_fullscreen_check->setChecked(b);
}

bool PrefGeneral::startInFullscreen()
{
    return start_fullscreen_check->isChecked();
}

#ifdef Q_OS_WIN
void PrefGeneral::setAvoidScreensaver(bool b)
{
    avoid_screensaver_check->setChecked(b);
}

bool PrefGeneral::avoidScreensaver()
{
    return avoid_screensaver_check->isChecked();
}

void PrefGeneral::setTurnScreensaverOff(bool b)
{
    turn_screensaver_off_check->setChecked(b);
}

bool PrefGeneral::turnScreensaverOff()
{
    return turn_screensaver_off_check->isChecked();
}
#else
void PrefGeneral::setDisableScreensaver(bool b)
{
    screensaver_check->setChecked(b);
}

bool PrefGeneral::disableScreensaver()
{
    return screensaver_check->isChecked();
}
#endif

void PrefGeneral::setBlackbordersOnFullscreen(bool b)
{
    blackborders_on_fs_check->setChecked(b);
}

bool PrefGeneral::blackbordersOnFullscreen()
{
    return blackborders_on_fs_check->isChecked();
}

void PrefGeneral::setScaleTempoFilter(Preferences::OptionState value)
{
    scaletempo_combo->setState(value);
}

Preferences::OptionState PrefGeneral::scaleTempoFilter()
{
    return scaletempo_combo->state();
}

void PrefGeneral::vo_combo_changed(int idx)
{
    qDebug("PrefGeneral::vo_combo_changed: %d", idx);
    bool visible = (vo_combo->itemData(idx).toString() == "user_defined");
    vo_user_defined_edit->setShown(visible);
    vo_user_defined_edit->setFocus();

#ifndef Q_OS_WIN
    bool vdpau_button_visible = (vo_combo->itemData(idx).toString() == "vdpau");
    vdpau_button->setShown(vdpau_button_visible);
#endif
}

void PrefGeneral::ao_combo_changed(int idx)
{
    qDebug("PrefGeneral::ao_combo_changed: %d", idx);
    bool visible = (ao_combo->itemData(idx).toString() == "user_defined");
    ao_user_defined_edit->setShown(visible);
    ao_user_defined_edit->setFocus();
}

#ifndef Q_OS_WIN
void PrefGeneral::on_vdpau_button_clicked()
{
    qDebug("PrefGeneral::on_vdpau_button_clicked");

    VDPAUProperties d(this);

    d.setffh264vdpau(vdpau.ffh264vdpau);
    d.setffmpeg12vdpau(vdpau.ffmpeg12vdpau);
    d.setffwmv3vdpau(vdpau.ffwmv3vdpau);
    d.setffvc1vdpau(vdpau.ffvc1vdpau);
    d.setffodivxvdpau(vdpau.ffodivxvdpau);

    d.setDisableFilters(vdpau.disable_video_filters);

    if (d.exec() == QDialog::Accepted) {
        vdpau.ffh264vdpau = d.ffh264vdpau();
        vdpau.ffmpeg12vdpau = d.ffmpeg12vdpau();
        vdpau.ffwmv3vdpau = d.ffwmv3vdpau();
        vdpau.ffvc1vdpau = d.ffvc1vdpau();
        vdpau.ffodivxvdpau = d.ffodivxvdpau();

        vdpau.disable_video_filters = d.disableFilters();
    }
}
#endif

void PrefGeneral::createHelp()
{
    clearHelp();

    addSectionTitle(tr("General"));

    setWhatsThis(mplayerbin_edit, tr("mplayer2 executable"),
                 tr("Here you must specify the mplayer2 "
                    "executable that SMPlayer2 will use.<br>"
                    "SMPlayer2 requires a recent version of mplayer2 (latest from git preferred)") + "<br><b>" +
                 tr("If this setting is wrong, SMPlayer2 won't be able to play "
                    "anything!") + "</b>");

    setWhatsThis(remember_all_check, tr("Remember settings"),
                 tr("Usually SMPlayer2 will remember the settings for each file you "
                    "play (audio track selected, volume, filters...). Disable this "
                    "option if you don't like this feature."));

    setWhatsThis(remember_time_check, tr("Remember time position"),
                 tr("If you check this option, SMPlayer2 will remember the last position "
                    "of the file when you open it again. This option works only with "
                    "regular files (not with DVDs, CDs, URLs...)."));

    setWhatsThis(filesettings_method_combo, tr("Method to store the file settings"),
                 tr("This option allows to change the way the file settings would be "
                    "stored. The following options are available:") + "<ul><li>" +
                 tr("<b>one ini file</b>: the settings for all played files will be "
                    "saved in a single ini file (%1)").arg(QString("<i>" + Paths::iniPath() + "/smplayer2.ini</i>")) + "</li><li>" +
                 tr("<b>multiple ini files</b>: one ini file will be used for each played file. "
                    "Those ini files will be saved in the folder %1").arg(QString("<i>" + Paths::iniPath() + "/file_settings</i>")) + "</li></ul>" +
                 tr("The latter method could be faster if there is info for a lot of files."));

    setWhatsThis(screenshot_edit, tr("Screenshots folder"),
                 tr("Here you can specify a folder where the screenshots taken by "
                    "SMPlayer2 will be stored. If the folder is not valid the "
                    "screenshot feature will be disabled."));

    setWhatsThis(close_on_finish_check, tr("Close when finished"),
                 tr("If this option is checked, the main window will be automatically "
                    "closed when the current file/playlist finishes."));

    setWhatsThis(pause_if_hidden_check, tr("Pause when minimized"),
                 tr("If this option is enabled, the file will be paused when the "
                    "main window is hidden. When the window is restored, playback "
                    "will be resumed."));

    // Video tab
    addSectionTitle(tr("Video"));

    setWhatsThis(vo_combo, tr("Video output driver"),
                 tr("Select the video output driver. %1 provides the best performance.")
#ifdef Q_OS_WIN
                 .arg("<b><i>directx</i></b>")
#else
                 .arg("<b><i>xv</i></b>")
#endif
                );

#ifndef Q_OS_WIN
    /*
    setWhatsThis(vdpau_filters_check, tr("Disable video filters when using vdpau"),
    	tr("Usually video filters won't work when using vdpau as video output "
           "driver, so it's wise to keep this option checked.") );
    */
#endif

    setWhatsThis(deinterlace_combo, tr("Deinterlace by default"),
                 tr("Select the deinterlace filter that you want to be used for new "
                    "videos opened.") + " " +
                 tr("<b>Note:</b> This option won't be used for TV channels."));

    setWhatsThis(zoom_spin, tr("Default zoom"),
                 tr("This option sets the default zoom which will be used for "
                    "new videos."));

    setWhatsThis(eq2_check, tr("Software video equalizer"),
                 tr("You can check this option if video equalizer is not supported by "
                    "your graphic card or the selected video output driver.<br>"
                    "<b>Note:</b> this option can be incompatible with some video "
                    "output drivers."));

    setWhatsThis(start_fullscreen_check, tr("Start videos in fullscreen"),
                 tr("If this option is checked, all videos will start to play in "
                    "fullscreen mode."));

    setWhatsThis(blackborders_on_fs_check, tr("Add black borders on fullscreen"),
                 tr("If this option is enabled, black borders will be added to the "
                    "image in fullscreen mode. This allows subtitles to be displayed "
                    "on the black borders.") /* + "<br>" +
 		tr("This option will be ignored if mplayer2 uses its own window, as "
           "some video drivers (like gl) are already able to display the "
           "subtitles automatically in the black borders.") */);

#ifdef Q_OS_WIN
    setWhatsThis(turn_screensaver_off_check, tr("Switch screensaver off"),
                 tr("This option switches the screensaver off just before starting to "
                    "play a file and switches it on when playback finishes. If this "
                    "option is enabled, the screensaver won't appear even if playing "
                    "audio files or when a file is paused."));

    setWhatsThis(avoid_screensaver_check, tr("Avoid screensaver"),
                 tr("When this option is checked, SMPlayer2 will try to prevent the "
                    "screensaver to be shown when playing a video file. The screensaver "
                    "will be allowed to be shown if playing an audio file or in pause "
                    "mode. This option only works if the SMPlayer2 window is in "
                    "the foreground."));
#else
    setWhatsThis(screensaver_check, tr("Disable screensaver"),
                 tr("Check this option to disable the screensaver while playing.<br>"
                    "The screensaver will enabled again when play finishes.")
                );
#endif

    // Audio tab
    addSectionTitle(tr("Audio"));

    setWhatsThis(ao_combo, tr("Audio output driver"),
                 tr("Select the audio output driver.")
#ifndef Q_OS_WIN
                 + " " +
                 tr("%1 is the recommended one. Try to avoid %2 and %3, they are slow "
                    "and can have an impact on performance.")
                 .arg("<b><i>alsa</i></b>")
                 .arg("<b><i>esd</i></b>")
                 .arg("<b><i>arts</i></b>")
#endif
                );

    setWhatsThis(audio_equalizer_check, tr("Enable the audio equalizer"),
                 tr("Check this option if you want to use the audio equalizer."));

    setWhatsThis(hwac3_check, tr("AC3/DTS pass-through S/PDIF"),
                 tr("Uses hardware AC3 passthrough.") + "<br>" +
                 tr("<b>Note:</b> none of the audio filters will be used when this "
                    "option is enabled."));

    setWhatsThis(gapless_audio_check, tr("Gapless audio"),
                 tr("Try to play consecutive files with no silence or disruption"
                    "at the point of file change.") + "<br>" +
                 tr("<b>Note:</b> The audio device is opened using parameters"
                    "chosen according to the first file played and is then kept"
                    "open for gapless playback. This means that if the first file"
                    "for example has a low samplerate then the following files may"
                    "get resampled to the same low samplerate, resulting in reduced"
                    "sound quality."));

    setWhatsThis(channels_combo, tr("Channels by default"),
                 tr("Requests the number of playback channels. mplayer2 "
                    "asks the decoder to decode the audio into as many channels as "
                    "specified. Then it is up to the decoder to fulfill the "
                    "requirement. This is usually only important when playing "
                    "videos with AC3 audio (like DVDs). In that case liba52 does "
                    "the decoding by default and correctly downmixes the audio "
                    "into the requested number of channels. "
                    "<b>Note</b>: This option is honored by codecs (AC3 only), "
                    "filters (surround) and audio output drivers (OSS at least)."));

    setWhatsThis(scaletempo_combo, tr("High speed playback without altering pitch"),
                 tr("Allows to change the playback speed without altering pitch. "));

    setWhatsThis(global_volume_check, tr("Global volume"),
                 tr("If this option is checked, the same volume will be used for "
                    "all files you play. If the option is not checked each "
                    "file uses its own volume.") + "<br>" +
                 tr("This option also applies for the mute control."));

    setWhatsThis(softvol_check, tr("Software volume control"),
                 tr("Check this option to use the software mixer, instead of "
                    "using the sound card mixer."));

    setWhatsThis(softvol_max_spin, tr("Max. Amplification"),
                 tr("Sets the maximum amplification level in percent (default: 110). "
                    "A value of 200 will allow you to adjust the volume up to a "
                    "maximum of double the current level. With values below 100 the "
                    "initial volume (which is 100%) will be above the maximum, which "
                    "e.g. the OSD cannot display correctly."));

    setWhatsThis(volnorm_check, tr("Volume normalization by default"),
                 tr("Maximizes the volume without distorting the sound."));

    setWhatsThis(autosync_check, tr("Audio/video auto synchronization"),
                 tr("Gradually adjusts the A/V sync based on audio delay "
                    "measurements."));

    setWhatsThis(mc_spin, tr("A-V sync correction"),
                 tr("Maximum A-V sync correction per frame (in seconds)"));

    addSectionTitle(tr("Preferred audio and subtitles"));

    setWhatsThis(audio_lang_edit, tr("Preferred audio language"),
                 tr("Here you can type your preferred language for the audio streams. "
                    "When a media with multiple audio streams is found, SMPlayer2 will "
                    "try to use your preferred language.<br>"
                    "This only will work with media that offer info about the language "
                    "of the audio streams, like DVDs or mkv files.<br>"
                    "This field accepts regular expressions. Example: <b>es|esp|spa</b> "
                    "will select the audio track if it matches with <i>es</i>, "
                    "<i>esp</i> or <i>spa</i>."));

    setWhatsThis(subtitle_lang_edit, tr("Preferred subtitle language"),
                 tr("Here you can type your preferred language for the subtitle stream. "
                    "When a media with multiple subtitle streams is found, SMPlayer2 will "
                    "try to use your preferred language.<br>"
                    "This only will work with media that offer info about the language "
                    "of the subtitle streams, like DVDs or mkv files.<br>"
                    "This field accepts regular expressions. Example: <b>es|esp|spa</b> "
                    "will select the subtitle stream if it matches with <i>es</i>, "
                    "<i>esp</i> or <i>spa</i>."));

    setWhatsThis(audio_track_spin, tr("Audio track"),
                 tr("Specifies the default audio track which will be used when playing "
                    "new files. If the track doesn't exist, the first one will be used. "
                    "<br><b>Note:</b> the <i>\"preferred audio language\"</i> has "
                    "preference over this option."));

    setWhatsThis(subtitle_track_spin, tr("Subtitle track"),
                 tr("Specifies the default subtitle track which will be used when "
                    "playing new files. If the track doesn't exist, the first one "
                    "will be used. <br><b>Note:</b> the <i>\"preferred subtitle "
                    "language\"</i> has preference over this option."));

}
