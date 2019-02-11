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

#include "NetworkDialog.hpp"
#include "WifiDialog.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "UIGlobals.hpp"
#include "Screen/Key.h"
#include "Language/Language.hpp"
#include "Form/Form.hpp"
#include "Form/ActionListener.hpp"
#include "Event/Timer.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Net/IpAddress.hpp"
#include "System.hpp"
#include <stdlib.h>

gcc_pure
static const TCHAR *
GetWifiToggleCaption()
{
  return IsKoboWifiOn() ? _T("Disable Wifi") : _T("Enable Wifi");
}

class NetworkWidget final
  : public RowFormWidget, ActionListener, Timer {
  enum Buttons {
    TOGGLE_WIFI = 0,
    WIFI,
    IPADDRESS,
  };

  Button *toggle_wifi_button, *wifi_button;

public:
  NetworkWidget(const DialogLook &look):RowFormWidget(look) {}

  void UpdateButtons();
  void UpdateIpAddress();

  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override;

  virtual void Unprepare() override {
    Timer::Cancel();
    RowFormWidget::Unprepare();
  }

  /* virtual methods from class Timer */
  virtual void OnTimer() {
    UpdateIpAddress();
  }

private:
  void ToggleWifi();

  /* virtual methods from class ActionListener */
  virtual void OnAction(int id) override;
};

void
NetworkWidget::UpdateIpAddress()
{
  char buffer[256];
  if (IpAddress::GetFormattedIpAddress(buffer) && strlen(buffer) > 3) {
    unsigned i = strlen(buffer);
    buffer[i - 1] = '\0';
    char caption[256];
    _stprintf(caption, _T("\nConnected\nLog in with username='root'\n  telnet %s or ftp %s"), buffer, buffer);
    SetMultiLineText(IPADDRESS, caption);
  } else
    SetMultiLineText(IPADDRESS, _T("\nNot connected"));
}

void
NetworkWidget::UpdateButtons()
{
  toggle_wifi_button->SetCaption(GetWifiToggleCaption());
  wifi_button->SetEnabled(IsKoboWifiOn());
}

void
NetworkWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  toggle_wifi_button = AddButton(GetWifiToggleCaption(),
                                 *this, TOGGLE_WIFI);

  wifi_button = AddButton(_("Select network"), *this, WIFI);

  AddMultiLine(_T(""));

  UpdateButtons();
  Timer::Schedule(1000);
}

void
NetworkWidget::ToggleWifi()
{
  if (!IsKoboWifiOn()) {
    KoboWifiOn();
  } else {
    KoboWifiOff();
  }

  UpdateButtons();
}

void
NetworkWidget::OnAction(int id)
{
  switch (id) {
  case TOGGLE_WIFI:
    ToggleWifi();
    break;

  case WIFI:
    ShowWifiDialog();
    break;
  }
}

void
ShowNetworkDialog()
{
  const DialogLook &look = UIGlobals::GetDialogLook();
  NetworkWidget widget(look);
  WidgetDialog dialog(look);
  dialog.CreateFull(UIGlobals::GetMainWindow(), _("Network"), &widget);
  dialog.AddSymbolButton(_T("_X"), mrOK);
  dialog.ShowModal();
  dialog.StealWidget();
}
