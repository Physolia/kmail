/* Basic Node for folder directory tree. Childs are KMFolder and KMFolderDir.
 * The owner of such nodes are usually objects of type KMFolderDir
 *
 * Author: Stefan Taferner <taferner@alpin.or.at>
 */
#ifndef kmfoldernode_h
#define kmfoldernode_h

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>

class KMFolderDir;

#define KMFolderNodeInherited QObject

class KMFolderNode: public QObject
{
  Q_OBJECT

public:
  KMFolderNode(KMFolderDir* parent, const char* name);
  virtual ~KMFolderNode();

  /** Is it a directory where mail folders are stored or is it a folder that
    contains mail ?
    Note that there are some kinds of mail folders like the type mh uses that
    are directories on disk but are handled as folders here. */
  virtual bool isDir(void) const;

  /** Returns ptr to owning directory object or NULL if none. This
    is just a wrapper for convenient access. */
  KMFolderDir* parent(void) const 
	{ return (KMFolderDir*)KMFolderNodeInherited::parent(); }

  /** Returns full path to the directory where this node is stored or NULL
   if the node has no parent. Example: if this object represents a folder
   ~joe/Mail/inbox then path() returns "/home/joe/Mail" and name() returns 
   "inbox". */
  virtual const QString& path(void) const;

  /** Name of the node. Also used as file name. */
  const QString& name(void) const { return mName; }
  void setName(const QString& aName) { mName = aName; }

  /** Label of the node for visualzation purposes. Default the same as
   the name. */
  virtual const QString label(void) const;

protected:
  QString mName;
};

typedef QList<KMFolderNode> KMFolderNodeList;


#endif /*kmfoldernode_h*/
