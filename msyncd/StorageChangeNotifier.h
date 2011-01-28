#ifndef STORAGECHANGENOTIFIER_H
#define STORAGECHANGENOTIFIER_H

#include <QObject>
#include <QHash>

namespace Buteo
{

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
    void loadNotifiers(PluginManager* aPluginManager,
                       const QStringList& aStorageNames);

    /*! Call this to start monitoring changes in storages
     *
     * @param list of storage names which can't be monitored
     * @return true if we can monitor all storages requested for
     * false otherwise
     */
    bool startListen(QStringList& aFailedStorages);

    /*! \brief call this to ignore taking action on
     * storage changes. Whether there was a change can
     * be determined by calling hasChanges() on the notifier plug-in
     * and startListen() can be called again
     */
    void stopListen();

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
    QHash<QString,StorageChangeNotifierPlugin*> iNotifierMap;
};

}

#endif
