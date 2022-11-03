#ifndef SYNCONCHANGE_H
#define SYNCONCHANGE_H

#include <QObject>
#include <QHash>
#include <QStringList>

namespace Buteo
{

class SyncProfile;
class StorageChangeNotifier;
class PluginManager;
class SyncOnChangeScheduler;

/*! \brief this class initiates a sync if there are changes
 * in storage(s) it's asked to monitor
 */
class SyncOnChange : public QObject
{
    Q_OBJECT

public:
    /*! \brief constructor
     */
    SyncOnChange();

    /*! \brief destructor
     */
    ~SyncOnChange();

    /*! \brief enable sync on change for a list of storages
     * for the interested profiles
     *
     * Destroys the profile objects when they are no longer needed
     *
     * @param aPluginManager Used to load SOC storage plug-ins
     * @param aSOCScheduler used to schedule SOC
     * @param aSOCStorageMap map of well-known storage name
     * to list of sync profiles insterested in SOC for that
     * storage
     * @param list of storage names for which SOC couldn't be enabled
     * @return false if SOC can't be enabled for one or more
     * storages
     */
    bool enable(const QHash<QString,QList<SyncProfile*> >& aSOCStorageMap,
                SyncOnChangeScheduler* aSOCScheduler,
                PluginManager* aPluginManager, QStringList& aFailedStorages);

    /*! If the storage change notifier plug-in's have already been loaded,
     * call this to re-enable sync on change. Handy to call after a disable.
     *
     * This also checks if there were changes when SOC was disabled, and notifies
     * if there were any
     */
    void enable();

    /*! \brief disable sync on change immediately, i.e stop listening
     * to change notifiers
     */
    void disable();

    /*! \brief Note the next change, and disable SOC if that happens
     */
    void disableNext();

    /*! \brief adds a profile to the list of profiles interested in soc for a specific storage
     *
     * @param aStorageName storage name
     * @param aProfile sync profile 
     */
    void addProfile(const QString& aStorageName, SyncProfile* aProfile);

public Q_SLOTS:
    /*! initiate sync for this storage
     */
    void sync(QString aStorageName);

private:
    /*! \brief destroys profile objects interested in SOC for this
     * storage
     */
    void cleanup(const QString& aStorageName);

    /*! \brief Get the names for storages for which SOC is desired
     *
     * @return list of storage names
     */
    QStringList getSOCStorageNames();

    StorageChangeNotifier* iStorageChangeNotifier;
    QHash<QString,QList<SyncProfile*> > iSOCStorageMap;
    SyncOnChangeScheduler* iSOCScheduler;
};

}

#endif
