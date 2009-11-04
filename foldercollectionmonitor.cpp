/*
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2009 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "foldercollectionmonitor.h"
#include <akonadi/changerecorder.h>
#include <akonadi/collection.h>
#include <akonadi/itemfetchscope.h>

FolderCollectionMonitor::FolderCollectionMonitor(QObject *parent)
  :QObject( parent )
{
  // monitor collection changes
  mMonitor = new Akonadi::ChangeRecorder( this );
  mMonitor->setCollectionMonitored( Akonadi::Collection::root() );
  mMonitor->fetchCollection( true );
  mMonitor->setAllMonitored( true );
  mMonitor->setMimeTypeMonitored( "message/rfc822" );
  mMonitor->setResourceMonitored( "akonadi_search_resource" ,  true );
  // TODO: Only fetch the envelope etc if possible.
  mMonitor->itemFetchScope().fetchFullPayload(true);
}

FolderCollectionMonitor::~FolderCollectionMonitor()
{
}

Akonadi::ChangeRecorder *FolderCollectionMonitor::monitor()
{
  return mMonitor;
}

void FolderCollectionMonitor::expireAllFolders(bool immediate )
{
    kDebug() << "AKONADI PORT: Need to implement it  " << Q_FUNC_INFO;
}

void FolderCollectionMonitor::compactAllFolders( bool immediate )
{
    kDebug() << "AKONADI PORT: Need to implement it  " << Q_FUNC_INFO;

}
