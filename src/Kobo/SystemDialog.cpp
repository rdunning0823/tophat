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

#include "SystemDialog.hpp"
#include "Kernel.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Dialogs/Message.hpp"
#include "UIGlobals.hpp"
#include "Language/Language.hpp"
#include "Form/ActionListener.hpp"
#include "Widget/RowFormWidget.hpp"
#include "System.hpp"
#include "Model.hpp"

class SystemWidget final
  : public RowFormWidget, ActionListener {
  enum Buttons {
    NICKEL,
    SWITCH_KERNEL,
    USB_STORAGE,
  };

public:
  SystemWidget(const DialogLook &look):RowFormWidget(look) {}

private:
  void SwitchKernel();
  void ExportUSBStorage();

  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override;

  /* virtual methods from class ActionListener */
  virtual void OnAction(int id) override;
};

void
SystemWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  bool otg_kernel = IsKoboOTGKernel();

  AddButton("start nickel", *this, NICKEL);
  SetRowEnabled(NICKEL, !otg_kernel);
  AddButton(otg_kernel ? "Disable USB host" : "Ensable USB host",
            *this, SWITCH_KERNEL);

  AddButton("Export USB storage", *this, USB_STORAGE);
  SetRowEnabled(USB_STORAGE, !otg_kernel);
}

inline void
SystemWidget::SwitchKernel()
{
#ifdef KOBO
  KoboModel model = DetectKoboModel();
  if (model != KoboModel::MINI &&
      model != KoboModel::GLO &&
      model != KoboModel::AURA2 &&
      ShowMessageBox(_T("This feature was designed for the Kobo Mini, Glo and Aura 2, but this is not one.  Use at your own risk.  Continue?"),
                     _T("USB-OTG"), MB_YESNO) != IDYES)
    return;

  char kobo_kernel[64];
  char *kernel_image = IsKoboOTGKernel()
    ? model_concat(kobo_kernel, sizeof(kobo_kernel),
		   "/opt/tophat/lib/kernel/", "/uImage.kobo")
    : model_concat(kobo_kernel, sizeof(kobo_kernel),
		   "/opt/tophat/lib/kernel/", "/uImage.otg");

  if (!KoboInstallKernel(kernel_image)) {
      ShowMessageBox(_T("Failed to activate kernel."), _("Error"), MB_OK);
      return;
  }

  KoboReboot();
#endif
}

inline void
SystemWidget::ExportUSBStorage()
{
  if (!KoboUmountData()) {
      ShowMessageBox(_T("Failed to unmount data partition."), _("Error"),
                     MB_OK);
      return;
  }

  if (!KoboExportUSBStorage()) {
      ShowMessageBox(_T("Failed to export data partition."), _("Error"),
                     MB_OK);
      KoboMountData();
      return;
  }

  ShowMessageBox(_T("Your PC has now access to the data partition until you close this dialog."),
                 _T("Export USB storage"),
                 MB_OK);

  KoboUnexportUSBStorage();
  KoboMountData();
}

void
SystemWidget::OnAction(int id)
{
  switch (id) {
  case NICKEL:
    KoboExecNickel();
    break;

  case SWITCH_KERNEL:
    SwitchKernel();
    break;

  case USB_STORAGE:
    ExportUSBStorage();
    break;
  }
}

void
ShowSystemDialog()
{
  const DialogLook &look = UIGlobals::GetDialogLook();
  SystemWidget widget(look);
  WidgetDialog dialog(look);
  dialog.CreateFull(UIGlobals::GetMainWindow(), "PC connect", &widget);
  dialog.AddSymbolButton(_T("_X"), mrOK);
  dialog.ShowModal();
  dialog.StealWidget();
}
