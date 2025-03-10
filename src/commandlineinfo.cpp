/*
   SPDX-FileCopyrightText: 2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "commandlineinfo.h"
#include "kmail_debug.h"
#include "kmail_options.h"
#include "messagecore/stringutil.h"
#include <QCommandLineParser>

CommandLineInfo::CommandLineInfo() = default;

CommandLineInfo::~CommandLineInfo() = default;

QDebug operator<<(QDebug d, const CommandLineInfo &t)
{
    d << "mCustomHeaders " << t.customHeaders();
    d << "mAttachURLs " << t.attachURLs();
    d << "mTo " << t.to();
    d << "mCc " << t.cc();
    d << "mBcc " << t.bcc();
    d << "mSubject " << t.subject();
    d << "mBody " << t.body();
    d << "mInReplyTo " << t.inReplyTo();
    d << "mReplyTo " << t.replyTo();
    d << "mIdentity " << t.identity();
    d << "mMessageFile " << t.messageFile();
    d << "mStartInTray " << t.startInTray();
    d << "mMailto " << t.mailto();
    d << "mCheckMail " << t.checkMail();
    d << "mViewOnly " << t.viewOnly();
    d << "mCalledWithSession " << t.calledWithSession();
    return d;
}

static QUrl makeAbsoluteUrl(const QString &str, const QString &cwd)
{
    return QUrl::fromUserInput(str, cwd, QUrl::AssumeLocalFile);
}

void CommandLineInfo::parseCommandLine(const QStringList &args, const QString &workingDir)
{
    // process args:
    QCommandLineParser parser;
    kmail_options(&parser);
    QStringList newargs;
    bool addAttachmentAttribute = false;
    for (const QString &argument : std::as_const(args)) {
        if (argument == QLatin1String("--attach")) {
            addAttachmentAttribute = true;
        } else {
            if (argument.startsWith(QLatin1String("--"))) {
                addAttachmentAttribute = false;
            }
            if (argument.contains(QLatin1Char('@')) || argument.startsWith(QLatin1String("mailto:"))) { // address mustn't be trade as a attachment
                addAttachmentAttribute = false;
            }
            if (addAttachmentAttribute) {
                newargs.append(QStringLiteral("--attach"));
                newargs.append(argument);
            } else {
                newargs.append(argument);
            }
        }
    }

    parser.process(newargs);
    if (parser.isSet(QStringLiteral("subject"))) {
        mSubject = parser.value(QStringLiteral("subject"));
        // if kmail is called with 'kmail -session abc' then this doesn't mean
        // that the user wants to send a message with subject "ession" but
        // (most likely) that the user clicked on KMail's system tray applet
        // which results in KMKernel::raise() calling "kmail kmail newInstance"
        // via D-Bus which apparently executes the application with the original
        // command line arguments and those include "-session ..." if
        // kmail/kontact was restored by session management
        if (mSubject == QLatin1String("ession")) {
            mSubject.clear();
            mCalledWithSession = true;
        } else {
            mMailto = true;
        }
    }

    const QStringList ccList = parser.values(QStringLiteral("cc"));
    if (!ccList.isEmpty()) {
        mMailto = true;
        mCc = ccList.join(QStringLiteral(", "));
    }

    const QStringList bccList = parser.values(QStringLiteral("bcc"));
    if (!bccList.isEmpty()) {
        mMailto = true;
        mBcc = bccList.join(QStringLiteral(", "));
    }

    if (parser.isSet(QStringLiteral("replyTo"))) {
        mMailto = true;
        mReplyTo = parser.value(QStringLiteral("replyTo"));
    }

    if (parser.isSet(QStringLiteral("msg"))) {
        mMailto = true;
        const QString file = parser.value(QStringLiteral("msg"));
        mMessageFile = makeAbsoluteUrl(file, workingDir);
    }

    if (parser.isSet(QStringLiteral("mBody"))) {
        mMailto = true;
        mBody = parser.value(QStringLiteral("mBody"));
    }

    const QStringList attachList = parser.values(QStringLiteral("attach"));
    if (!attachList.isEmpty()) {
        mMailto = true;
        for (const QString &attach : attachList) {
            if (!attach.isEmpty()) {
                mAttachURLs.append(makeAbsoluteUrl(attach, workingDir));
            }
        }
    }

    mCustomHeaders = parser.values(QStringLiteral("header"));

    if (parser.isSet(QStringLiteral("composer"))) {
        mMailto = true;
    }

    if (parser.isSet(QStringLiteral("check"))) {
        mCheckMail = true;
    }

    if (parser.isSet(QStringLiteral("startintray"))) {
        mStartInTray = true;
    }

    if (parser.isSet(QStringLiteral("identity"))) {
        mIdentity = parser.value(QStringLiteral("identity"));
    }

    if (parser.isSet(QStringLiteral("view"))) {
        mViewOnly = true;
        const QString filename = parser.value(QStringLiteral("view"));
        mMessageFile = QUrl::fromUserInput(filename, workingDir);
    }

    if (!mCalledWithSession) {
        // only read additional command line arguments if kmail/kontact is
        // not called with "-session foo"
        const QStringList lstPositionalArguments = parser.positionalArguments();
        for (const QString &arg : lstPositionalArguments) {
            if (arg.startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive)) {
                const QUrl urlDecoded(QUrl::fromPercentEncoding(arg.toUtf8()));
                const QList<QPair<QString, QString>> values = MessageCore::StringUtil::parseMailtoUrl(urlDecoded);
                QString previousKey;
                for (int i = 0; i < values.count(); ++i) {
                    const QPair<QString, QString> element = values.at(i);
                    const QString key = element.first.toLower();
                    if (key == QLatin1String("to")) {
                        if (!element.second.isEmpty()) {
                            mTo += element.second + QStringLiteral(", ");
                        }
                        previousKey.clear();
                    } else if (key == QLatin1String("cc")) {
                        if (!element.second.isEmpty()) {
                            mCc += element.second + QStringLiteral(", ");
                        }
                        previousKey.clear();
                    } else if (key == QLatin1String("bcc")) {
                        if (!element.second.isEmpty()) {
                            mBcc += element.second + QStringLiteral(", ");
                        }
                        previousKey.clear();
                    } else if (key == QLatin1String("subject")) {
                        mSubject = element.second;
                        previousKey.clear();
                    } else if (key == QLatin1String("mBody")) {
                        mBody = element.second;
                        previousKey = key;
                    } else if (key == QLatin1String("in-reply-to")) {
                        mInReplyTo = element.second;
                        previousKey.clear();
                    } else if (key == QLatin1String("attachment") || key == QLatin1String("attach")) {
                        if (!element.second.isEmpty()) {
                            mAttachURLs << makeAbsoluteUrl(element.second, workingDir);
                        }
                        previousKey.clear();
                    } else {
                        qCWarning(KMAIL_LOG) << "unknown key" << key;
                        // Workaround: https://bugs.kde.org/show_bug.cgi?id=390939
                        // QMap<QString, QString> parseMailtoUrl(const QUrl &url) parses correctly url
                        // But if we have a "&" unknown key we lost it.
                        if (previousKey == QLatin1String("mBody")) {
                            mBody += QLatin1Char('&') + key + QLatin1Char('=') + element.second;
                        }
                        // Don't clear previous key.
                    }
                }
            } else {
                QUrl url(arg);
                if (url.isValid() && !url.scheme().isEmpty()) {
                    mAttachURLs += url;
                } else {
                    mTo += arg + QStringLiteral(", ");
                }
            }
            mMailto = true;
        }
        if (!mTo.isEmpty()) {
            // cut off the superfluous trailing ", "
            mTo.chop(2);
        }
    }
}

QStringList CommandLineInfo::customHeaders() const
{
    return mCustomHeaders;
}

QList<QUrl> CommandLineInfo::attachURLs() const
{
    return mAttachURLs;
}

QString CommandLineInfo::to() const
{
    return mTo;
}

QString CommandLineInfo::cc() const
{
    return mCc;
}

QString CommandLineInfo::bcc() const
{
    return mBcc;
}

QString CommandLineInfo::subject() const
{
    return mSubject;
}

QString CommandLineInfo::body() const
{
    return mBody;
}

QString CommandLineInfo::inReplyTo() const
{
    return mInReplyTo;
}

QString CommandLineInfo::replyTo() const
{
    return mReplyTo;
}

QString CommandLineInfo::identity() const
{
    return mIdentity;
}

QUrl CommandLineInfo::messageFile() const
{
    return mMessageFile;
}

bool CommandLineInfo::startInTray() const
{
    return mStartInTray;
}

bool CommandLineInfo::mailto() const
{
    return mMailto;
}

bool CommandLineInfo::checkMail() const
{
    return mCheckMail;
}

bool CommandLineInfo::viewOnly() const
{
    return mViewOnly;
}

bool CommandLineInfo::calledWithSession() const
{
    return mCalledWithSession;
}
