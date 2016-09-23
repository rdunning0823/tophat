/*
Copyright_License {

  Top Hat Soaring Glide Computer - http://www.tophatsoaring.org/
  Copyright (C) 2000-2016 The Top Hat Soaring Project
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
#include "Form/DataField/File.hpp"
#include "Form/DataField/Float.hpp"
#include "Language/Language.hpp"
#include "Event/Timer.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "UISettings.hpp"
#include "Interface.hpp"

enum Buttons {
  START,
  STOP,
  REWIND,
  FAST_FORWARD,
};

class ReplayControlWidget final
  : public RowFormWidget, ActionListener, DataFieldListener, private Timer {
  enum Controls {
    FILE,
    RATE,
  };
  Button *play_pause_button;
  Button *stop_button;
  Button *rewind_button;
  Button *fast_forward_button;
  WidgetDialog *dialog;

public:
  ReplayControlWidget(const DialogLook &look)
    :RowFormWidget(look), dialog(nullptr) {}

  void CreateButtons(WidgetDialog &dialog) {
    play_pause_button = dialog.AddSymbolButton(_(">"), *this, START);
    stop_button = dialog.AddSymbolButton(_("[]"), *this, STOP);
    rewind_button = dialog.AddSymbolButton(_T("<<"), *this, REWIND);
    fast_forward_button = dialog.AddSymbolButton(_T(">>"), *this, FAST_FORWARD);
  }

  void SetDialog(WidgetDialog &_dialog) {
    dialog = &_dialog;
  }

private:
  void OnStopClicked();
  void OnStartClicked();
  void OnRewindClicked();
  void OnFastForwardClicked();
  void UpdateButtons();
  void UpdateDialogTitle();
  /* update replay control */
  bool StartReplay();
  /* update replay control */
  void SetReplayRate(fixed rate);
  /* is replay in fast forward mode? */
  bool CheckFastForward();
  /* updates play state when replay object finishes the fast forward mode */
  void OnFastForwardDone();

  void FastForwardCancel();
  /* exits Replay::PlayState::PAUSED */
  void Resume();

private:
  virtual void OnTimer() override;

public:
  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override;
  virtual void Unprepare() override;

private:
  /* virtual methods from ActionListener */
  virtual void OnAction(int id) override;

  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df) override;
};

void
ReplayControlWidget::UpdateButtons()
{
  play_pause_button->SetEnabled(!replay->IsNoFile());
  fast_forward_button->SetEnabled(replay->IsPaused() || replay->IsPlaying());
  rewind_button->SetEnabled(replay->IsPaused() || replay->IsPlaying());
  stop_button->SetEnabled(replay->IsPaused() || replay->IsPlaying()
                          || replay->IsFastForward());

  switch(replay->GetPlayState()) {
  case Replay::PlayState::NOFILE:
  case Replay::PlayState::NOTSTARTED:
    play_pause_button->SetCaption(_(">"));
    break;
  case Replay::PlayState::PLAYING:
    play_pause_button->SetCaption(_("||"));
    break;
  case Replay::PlayState::PAUSED:
    play_pause_button->SetCaption(_(">"));
    break;
  case Replay::PlayState::FASTFORWARD:
    break;
  }
}

void
ReplayControlWidget::UpdateDialogTitle()
{

  StaticString<50> header;
  header = _("Replay");
  if (replay->IsActive() && positive(replay->GetTime())) {
    TCHAR buffer[32];
    FormatTime(buffer, replay->GetTime());
    header.AppendFormat(_T(" %s"), buffer);
    if (replay->CheckFastForward())
      header.AppendFormat(_T(" %s"), _("FF"));

    if (replay->IsPaused())
      header.AppendFormat(_T(" %s"), _("Paused"));

  }
  assert(dialog != nullptr);
  dialog->SetCaption(header.c_str());
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
    AddFile(_("File"),
            _("Name of file to replay.  Can be an IGC file (.igc), a raw NMEA log file (.nmea), or if blank, runs the demo."),
            nullptr,
            _T("*.nmea\0*.igc\0"),
            true);
  ((FileDataField *)file->GetDataField())->SetReverseSort();
  ((FileDataField *)file->GetDataField())->Lookup(replay->GetFilename());
  file->RefreshDisplay();

  AddFloat(_("Rate"),
           _("Time acceleration of replay. Set to 0 for pause, 1 for normal real-time replay."),
           _T("%.1f x"), _T("%.1f"),
           fixed(0), fixed(20), fixed(1), false,
           replay->GetTimeScale(),
           this);
  Timer::Schedule(500);
}

bool
ReplayControlWidget::CheckFastForward()
{
  return replay->CheckFastForward();
}

void
ReplayControlWidget::OnTimer()
{
  if (!replay->IsActive())
    replay->SetPlayState(Replay::PlayState::NOTSTARTED);

  UpdateButtons();
  UpdateDialogTitle();

  if (replay->IsFastForward() && !CheckFastForward())
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
  const FileDataField &df = (const FileDataField &)
    GetDataField(FILE);
  const TCHAR *path = df.GetPathFile();
  if (replay->Start(path)) {
    return true;
  } else {
    replay->SetPlayState(Replay::PlayState::NOFILE);
  }
  return false;
}

inline void
ReplayControlWidget::OnStopClicked()
{
  replay->Stop();
}

inline void
ReplayControlWidget::Resume()
{
  replay->SetPlayState(Replay::PlayState::PLAYING);
}

inline void
ReplayControlWidget::OnStartClicked()
{
  switch(replay->GetPlayState()) {
  case Replay::PlayState::NOFILE:
    break;
  case Replay::PlayState::NOTSTARTED:
    if (!StartReplay())
      ShowMessageBox(_("Could not open IGC file!"),
                     _("Replay"), MB_OK | MB_ICONINFORMATION);
    break;
  case Replay::PlayState::FASTFORWARD:
    FastForwardCancel();
    replay->SetPlayState(Replay::PlayState::PAUSED);
    break;
  case Replay::PlayState::PLAYING:
    replay->SetPlayState(Replay::PlayState::PAUSED);
    break;
  case Replay::PlayState::PAUSED:
    Resume();
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

  case REWIND:
    OnRewindClicked();
    break;

  }
}

inline void
ReplayControlWidget::OnRewindClicked()
{
  if (replay->IsPaused())
    Resume();

  if (replay->Rewind(fixed(10 * 60))) {
    replay->SetPlayState(Replay::PlayState::FASTFORWARD);
  }
}

inline void
ReplayControlWidget::OnFastForwardClicked()
{
  if (replay->IsPaused())
    Resume();

  if (replay->FastForward(fixed(10 * 60))) {
    replay->SetPlayState(Replay::PlayState::FASTFORWARD);
  }
}

inline void
ReplayControlWidget::OnFastForwardDone()
{
  replay->SetPlayState(Replay::PlayState::PLAYING);
}

inline void
ReplayControlWidget::FastForwardCancel()
{
  replay->FastForwardCancel();
}

void
ReplayControlWidget::OnModified(DataField &_df)
{
  if (&_df == &GetDataField(RATE)) {
    const DataFieldFloat &df = (const DataFieldFloat &)_df;
    SetReplayRate(df.GetAsFixed());
  } else if (&_df == &GetDataField(FILE)) {
    OnStopClicked();
    replay->SetPlayState(Replay::PlayState::NOTSTARTED);
  }
}

void
ShowReplayDialog()
{
  const DialogLook &look = UIGlobals::GetDialogLook();
  ReplayControlWidget *widget = new ReplayControlWidget(look);
  WidgetDialog dialog(look);
  dialog.CreatePopup(UIGlobals::GetMainWindow(), _("Replay"), widget);
  dialog.AddSymbolButton(_("_X"), mrOK);
  widget->CreateButtons(dialog);
  widget->SetDialog(dialog);

  UISettings &ui_settings = CommonInterface::SetUISettings();
  ui_settings.replay_dialog_visible = true;
  dialog.ShowModal();
  ui_settings.replay_dialog_visible = false;
}
