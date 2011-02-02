#ifndef SYNCONCHANGESCHEDULER_H
#define SYNCONCHANGESCHEDULER_H

#include <QObject>
#include <QHash>
#include <QStringList>

#include "SyncScheduler.h"

/*! \brief default timeout (in seconds) for initiating SOC - 5 mins
 */
const quint32 DEFAULT_SOC_AFTER_TIME(5*60);

namespace Buteo
{

class SyncProfile;

class SyncOnChangeScheduler : public SyncScheduler
{
    Q_OBJECT;

public:
    /*! \brief constructor
     */
    SyncOnChangeScheduler();

    /*! \brief destructor
     */
    ~SyncOnChangeScheduler();

    /*! \brief Call this method to schedule SOC for a profile
     *
     * There are 3 scheduling criteria - SOC after info from the
     * profile, default SOC after and sync now.
     *
     * The profile is first checked for sync on change after time, which
     * should be specified in seconds (0 means sync now). If none is specified
     * then we use a default of DEFAULT_SOC_AFTER_TIME
     *
     * If the profile has already been added and if it's SOC is scheduled,
     * calling this method again will just use the previous schedule, and in
     * this case the method will return false.
     *
     * Once the SOC is initiated (by sending a syncNow signal), the profile is
     * removed automatically
     * 
     * @param aProfile pointer to sync profile
     * @return true if SOC could be scheduled, false otherwise
     */
    bool addProfile(const SyncProfile* aProfile);

    /*! \brief call this method to disable SOC that has been scheduled
     * for a certain profile
     *
     * @param aProfileName name of the profile
     */
    void removeProfile(const QString &aProfileName);

    /*! \brief call this method to set your own default timeout for
     * the timeout schedule criterion
     *
     * @param aTime time in seconds
     */
    void setDefaultSOCAfterTime(const quint32& aTime);

private Q_SLOTS:
    /*! \brief slot to initiate sync when timeout criterion is being used
     * and the timeout occurs
     *
     * @param aProfile sync profile
     */
    void sync(const SyncProfile* aProfile);

private:
    quint32 iDefaultSOCAfterTime;
    QStringList iSOCProfileNames;
};

class SyncOnChangeTimer : public QObject
{
    Q_OBJECT

public:
    /*! \brief constructor
     */
    SyncOnChangeTimer(const SyncProfile* aProfile, const quint32& aTimeout);

    /*! \brief destructor
     */
    ~SyncOnChangeTimer();

    /*! \brief fire the timer
     */
    void fire();

Q_SIGNALS:
    /*! \brief emit this signal when the timeout occurs
     *
     * @param aProfile sync profile
     */
    void timeout(const SyncProfile* aProfile);

private Q_SLOTS:
    /*! \brief slot corresponding to timeout
     */
    void onTimeout();

private:
    const SyncProfile* iSyncProfile;
    quint32 iTimeout;
};

}

#endif
