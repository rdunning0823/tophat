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

#ifndef TIMECONFIGPANEL_HPP
#define TIMECONFIGPANEL_HPP

#include "Widget/RowFormWidget.hpp"
#include "Form/DataField/Listener.hpp"
#include "UIGlobals.hpp"


class TimeConfigPanel final
  : public RowFormWidget, DataFieldListener {
protected:
  /* reference to parent form */
  //WndForm *_form;

public:
  TimeConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
/*
  void SetForm(WndForm *_form) {
    assert(_form != nullptr);
    form = _form;
  }
*/

  void SetLocalTime(RoughTimeDelta utc_offset);

  /* methods from Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;

private:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df) override;
};
Widget *
CreateTimeConfigPanel();

#endif /* TIMECONFIGPANEL_HPP */
