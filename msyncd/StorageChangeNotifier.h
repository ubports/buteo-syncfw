#ifndef STORAGECHANGENOTIFIER_H
#define STORAGECHANGENOTIFIER_H

#include <QObject>
#include <QHash>

namespace Buteo {

class StorageChangeNotifierPlugin;
class PluginManager;

/*! \brief Notifies about changes in storages
 * that it's asked to monitor
 */
class StorageChangeNotifier : public QObject
{
    Q_OBJECT

public:
    /*! \brief constructor
     */
    StorageChangeNotifier();

    /*! \brief destructor
     */
    ~StorageChangeNotifier();

    /*! \brief load all implemented storage change notifier plug-in's
     *
     * @param aPluginManager used to load SOC storage plugins
     * @param aStorageNames list of storages we wan't to monitor
     */
    void loadNotifiers(PluginManager *aPluginManager,
                       const QStringList &aStorageNames);

    /*! Call this to start monitoring changes in storages
     *
     * @param list of storage names which can't be monitored
     * @return true if we can monitor all storages requested for
     * false otherwise
     */
    bool startListen(QStringList &aFailedStorages);

    /*! \brief call this to ignore taking action on
     * storage changes. Whether there was a change can
     * be determined by calling hasChanges() on the notifier plug-in
     * and startListen() can be called again
     *
     * @param disableAfterNextChange if set to true, we stop listening
     * to change notifiers after they've notified about the next change.
     * This is useful for eg to note that a change did occur during a d2d
     * sync (during which we disable SOC), but we don't want to get notified
     * for each batch change, the one notification lets itself be known to us
     * when we call checkForChanges()
     */
    void stopListen(bool disableAfterNextChange = false);

    /*! Manually check and notify changes in storage
     */
    void checkForChanges();

private Q_SLOTS:
    /*! \brief process a storage change notification
     */
    void storageChanged();

Q_SIGNALS:
    /*! emit this signal if a storage changed
     *
     * @param storageName name of the storage that changed
     */
    void storageChange(QString aStorageName);

private:
    QHash<QString, StorageChangeNotifierPlugin *> iNotifierMap;
    PluginManager *iPluginManager;
};

}

#endif
