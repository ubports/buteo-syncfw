#ifndef STORAGECHANGENOTIFIERPLUGIN_H
#define STORAGECHANGENOTIFIERPLUGIN_H

#include <QObject>
#include <QString>

namespace Buteo
{

/*! \brief Implement this class to notify about changes in
 * a specific storage - contacts/calendar/sms, or even custom
 * ones like a facebook storage, if there's such a storage on
 * the device
 */
class StorageChangeNotifierPlugin : public QObject
{
    Q_OBJECT

public: 
    /*! \brief constructor
     * @param aStorageName pass the well known sync storage name
     */
    StorageChangeNotifierPlugin(const QString& aStorageName):
    iStorageName(aStorageName){}

    /*! \brief destructor
     */
    virtual ~StorageChangeNotifierPlugin(){};

    /*! \brief the name should be a well-known name
     * which buteo sync-fw knows about as a storage that
     * could be synced, for eg hcontacts for contacts storage
     *
     * @return well-known storage name
     */
    virtual QString name() const = 0;

    /*! \brief Check if this storage has changes since the
     * last time it was asked for the same
     *
     * @return true if there are changes, false otherwise
     */
    virtual bool hasChanges() const = 0;

    /*! Call this after the change notification has been
     * received, either via a signal or by calling hasChanges()
     * manually
     */
    virtual void changesReceived() = 0; 

    /*! \brief Enable listening to storage changes
     */
    virtual void enable() = 0;

    /*! \brief Disable listening to storage changes
     *
     * @param disableAfterNextChange if set to true, then we
     * disable listening only if the next change occurs and
     * if enable() hasn't been called before this change is receivied.
     * This helps to get one notification in case items are added in batches.
     */
    virtual void disable(bool disableAfterNextChange = false) = 0;

Q_SIGNALS:
    /*! \brief emit this signal when there's a change in this
     * storage. It's upto the plug-in when and how frequently
     * it wants to emit this signal
     */
    void storageChange();

protected:
    QString iStorageName;
};

}

#endif
