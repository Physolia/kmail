/*
 * This file is part of KMail.
 * SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QByteArray>
#include <QList>
class CodecManager
{
public:
    /**
      Returns the CodecManager instance.
    */
    static CodecManager *self();

    /**
      A list of preferred charsets to use when composing messages.
    */
    [[nodiscard]] QList<QByteArray> preferredCharsets() const;

    /**
      Re-read the preferred charsets from settings.
      TODO KConfig XT would probably make this unnecessary
    */
    void updatePreferredCharsets();

private:
    // Singleton.  The only instance lives in sInstance->instance
    CodecManager();

    QList<QByteArray> mPreferredCharsets;
};
