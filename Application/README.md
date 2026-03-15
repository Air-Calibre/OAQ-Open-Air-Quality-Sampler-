## Prerequisites

Ensure the following are installed before running the project:

- Node.js
- npm / yarn / pnpm
- Java Development Kit (JDK) 17
- Android Studio with Android SDK
- Expo CLI (`npm install -g expo`)
- Git

For Android development:
- Android emulator with Bluetooth support or a physical Android device with USB debugging enabled through developer options after enabling them by clicking build number ~7 times on your phone.

> This project uses Expo Dev Client (not Expo Go).
> You still need Expo Go installed on your Phone.

Installation & Running

1) Clone the repository

```cmd
git clone https://github.com/Air-Calibre/App-dev.git
cd App-dev
```

2) Install dependencies:

```cmd
npm install
```

3) Ensure an Android device or emulator is connected:

```cmd
adb devices
```

You should see your device displayed here. If it says UNAUTHORIZED make sure USB debugging is on and when you get the prompt to allow your PC to access your device allow it.

4) Prebuild native projects:
```cmd
npx expo prebuild
```

5) Build and run the app on Android:

```cmd
npx expo run:android
```

6) Choose android:
```cmd
a
```


