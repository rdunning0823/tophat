/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "ReplayDialog.hpp"
#include "Dialogs/Message.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Form/ActionListener.hpp"
#include "Form/Button.hpp"
#include "Form/DataField/Listener.hpp"
#include "UIGlobals.hpp"
#include "Components.hpp"
#include "Replay/Replay.hpp"
#include "Form/DataField/FileReader.hpp"
#include "Form/DataField/Float.hpp"
#include "Language/Language.hpp"
#include "Event/Timer.hpp"

enum Buttons {
  START,
  STOP,
  FAST_FORWARD,
};

class ReplayControlWidget final
  : public RowFormWidget, ActionListener, DataFieldListener, private Timer {
  enum Controls {
    FILE,
    RATE,
  };

  enum PlayState {
    NOFILE,
    NOTSTARTED,
    PLAYING,
    PAUSED,
  };

  PlayState play_state;
  WndButton *play_pause_button;
  fixed user_speed;

public:
  ReplayControlWidget(const DialogLook &look)
    :RowFormWidget(look), play_state(PlayState::NOFILE), user_speed(fixed(0)) {}

  void CreateButtons(WidgetDialog &dialog) {
    play_pause_button = dialog.AddButton(_("Start"), *this, START);
    dialog.AddButton(_("Reset"), *this, STOP);
    dialog.AddButton(_T("+10'"), *this, FAST_FORWARD);
  }

private:
  void OnStopClicked();
  void OnStartClicked();
  void OnFastForwardClicked();
  void UpdateButtons();
  /* update replay control */
  bool StartReplay();
  /* update replay control */
  void SetReplayRate(fixed rate);

private:
  virtual void OnTimer() override;

public:
  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override;
  virtual void Unprepare();

private:
  /* virtual methods from ActionListener */
  virtual void OnAction(int id) override;

  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df) override;
};

void
ReplayControlWidget::UpdateButtons()
{
  play_pause_button->SetEnabled(play_state != PlayState::NOFILE);

  switch(play_state) {
  case PlayState::NOFILE:
  case PlayState::NOTSTARTED:
    play_pause_button->SetCaption(_("Start"));
    break;
  case PlayState::PLAYING:
    play_pause_button->SetCaption(_("Pause"));
    break;
  case PlayState::PAUSED:
    play_pause_button->SetCaption(_("Resume"));
    break;
  }
}

void
ReplayControlWidget::Unprepare()
{
  Timer::Cancel();
  RowFormWidget::Unprepare();

}

void
ReplayControlWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  auto *file =
    AddFileReader(_("File"),
                  _("Name of file to replay.  Can be an IGC file (.igc), a raw NMEA log file (.nmea), or if blank, runs the demo."),
                  nullptr,
                  _T("*.nmea\0*.igc\0"),
                  true,
                  this);
  ((DataFieldFileReader *)file->GetDataField())->SetReverseSort();
  ((DataFieldFileReader *)file->GetDataField())->Lookup(replay->GetFilename());
  file->RefreshDisplay();

  user_speed = replay->GetTimeScale();

  AddFloat(_("Rate"),
           _("Time acceleration of replay. Set to 0 for pause, 1 for normal real-time replay."),
           _T("%.0f x"), _T("%.0f"),
           fixed(0), fixed(10), fixed(1), false,
           user_speed,
           this);
  Timer::Schedule(500);
}

void
ReplayControlWidget::OnTimer()
{
  UpdateButtons();
  UpdateDialogTitle();

  if (play_state == PlayState::FASTFORWARD && !CheckFastForward())
    OnFastForwardDone();
}

void
ReplayControlWidget::SetReplayRate(fixed rate)
{
  replay->SetTimeScale(rate);
}

bool
ReplayControlWidget::StartReplay()
{
  const DataFieldFileReader &df = (const DataFieldFileReader &)
    GetDataField(FILE);
  const TCHAR *path = df.GetPathFile();
  if (replay->Start(path)) {
    SetReplayRate(user_speed);
    play_state = PlayState::PLAYING;
    return true;
  } else {
    play_state = PlayState::NOFILE;
  }
  return false;
}

inline void
ReplayControlWidget::OnStopClicked()
{
  replay->Stop();
  play_state = PlayState::NOTSTARTED;
}

inline void
ReplayControlWidget::OnStartClicked()
{
  switch(play_state) {
  case PlayState::NOFILE:
    break;
  case PlayState::NOTSTARTED:
    if (!StartReplay())
      ShowMessageBox(_("Could not open IGC file!"),
                     _("Replay"), MB_OK | MB_ICONINFORMATION);
    break;
  case PlayState::PLAYING:
    play_state = PlayState::PAUSED;
    SetReplayRate(fixed(0));
    break;
  case PlayState::PAUSED:
    SetReplayRate(user_speed);
    play_state = PlayState::PLAYING;
    break;
  }
}

void
ReplayControlWidget::OnAction(int id)
{
  switch (id) {
  case START:
    OnStartClicked();
    break;

  case STOP:
    OnStopClicked();
    break;

  case FAST_FORWARD:
    OnFastForwardClicked();
    break;
  }
}

inline void
ReplayControlWidget::OnFastForwardClicked()
{
  replay->FastForward(fixed(10 * 60));
}

void
ReplayControlWidget::OnModified(DataField &_df)
{
  if (&_df == &GetDataField(RATE)) {
    const DataFieldFloat &df = (const DataFieldFloat &)_df;
    user_speed = df.GetAsFixed();
    SetReplayRate(user_speed);
  } else if (&_df == &GetDataField(FILE)) {
    OnStopClicked();
    play_state = PlayState::NOTSTARTED;
  }
}

void
ShowReplayDialog()
{
  const DialogLook &look = UIGlobals::GetDialogLook();
  ReplayControlWidget *widget = new ReplayControlWidget(look);
  WidgetDialog dialog(look);
  dialog.CreatePopup(UIGlobals::GetMainWindow(), _("Replay"), widget);
  dialog.AddButton(_("Close"), mrOK);
  widget->CreateButtons(dialog);

  dialog.ShowModal();
}
