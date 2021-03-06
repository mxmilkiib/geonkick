/**
 * File name: right_bar.cpp
 * Project: Geonkick (A kick synthesizer)
 *
 * Copyright (C) 2020 Iurie Nistor <http://geontime.com>
 *
 * This file is part of Geonkick.
 *
 * GeonKick is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "right_bar.h"
#include "geonkick_button.h"

#include <RkContainer.h>

RK_DECLARE_IMAGE_RC(controls_button);
RK_DECLARE_IMAGE_RC(kit_button);
RK_DECLARE_IMAGE_RC(presets_button);

RightBar::RightBar(GeonkickWidget *parent)
        : GeonkickWidget(parent)
{
        constexpr int barWidth = 15;
        setFixedSize(barWidth, parent->height());

        auto container = new RkContainer(this, Rk::Orientation::Vertical);
        container->setSize(size());

        auto controlsButton = new GeonkickButton(this);
        controlsButton->setCheckable(true);
        controlsButton->setSize(15, 119);
        controlsButton->setUnpressedImage(RkImage(15, 119, RK_IMAGE_RC(controls_button)));
        RK_ACT_BIND(controlsButton, toggled, RK_ACT_ARGS(bool b), this, showControls());
        container->addSpace(405);
        container->addWidget(controlsButton);

        auto kitSettingsButton = new GeonkickButton(this);
        kitSettingsButton->setCheckable(true);
        kitSettingsButton->setSize(15, 119);
        kitSettingsButton->setUnpressedImage(RkImage(15, 119, RK_IMAGE_RC(kit_button)));
        RK_ACT_BIND(kitSettingsButton, toggled, RK_ACT_ARGS(bool b), this, showKit());
        container->addSpace(12);
        container->addWidget(kitSettingsButton);

        auto presetsButton = new GeonkickButton(this);
        presetsButton->setType(RkButton::ButtonType::ButtonPush);
        presetsButton->setSize(15, 73);
        presetsButton->setUnpressedImage(RkImage(presetsButton->size(), RK_IMAGE_RC(presets_button)));
        RK_ACT_BIND(presetsButton, pressed, RK_ACT_ARGS(), this, showPresets());
        container->addSpace(12);
        container->addWidget(presetsButton);

}
