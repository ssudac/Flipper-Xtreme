#pragma once

typedef enum {
    DesktopMainEventOpenLockMenu,
    DesktopMainEventOpenArchive,
    DesktopMainEventOpenFavoritePrimary,
    DesktopMainEventOpenFavoriteSecondary,
    DesktopMainEventOpenMenu,
    DesktopMainEventOpenGames,
    DesktopMainEventOpenDebug,
    DesktopMainEventOpenPassport,
    DesktopMainEventOpenPowerOff,
    DesktopMainEventLock,

    DesktopMainEventOpenSnake,
    DesktopMainEventOpen2048,
    DesktopMainEventOpenZombiez,
    DesktopMainEventOpenTetris,
    DesktopMainEventOpenDOOM,
    DesktopMainEventOpenDice,
    DesktopMainEventOpenArkanoid,
    DesktopMainEventOpenHeap,
    DesktopMainEventOpenSubRemote,
    DesktopMainEventOpenClock,

    DesktopLockedEventUnlocked,
    DesktopLockedEventUpdate,
    DesktopLockedEventShowPinInput,

    DesktopPinInputEventResetWrongPinLabel,
    DesktopPinInputEventUnlocked,
    DesktopPinInputEventUnlockFailed,
    DesktopPinInputEventBack,

    DesktopPinTimeoutExit,

    DesktopDebugEventDeed,
    DesktopDebugEventWrongDeed,
    DesktopDebugEventSaveState,
    DesktopDebugEventExit,

    DesktopLockMenuEventSettings,
    DesktopLockMenuEventLock,
    DesktopLockMenuEventLockPin,
    DesktopLockMenuEventLockPinOff,
    DesktopLockMenuEventXtreme,

    DesktopAnimationEventCheckAnimation,
    DesktopAnimationEventNewIdleAnimation,
    DesktopAnimationEventInteractAnimation,

    DesktopSlideshowCompleted,
    DesktopSlideshowPoweroff,

    // Global events
    DesktopGlobalBeforeAppStarted,
    DesktopGlobalAfterAppFinished,
    DesktopGlobalAutoLock,
} DesktopEvent;
