import { Buffer } from "buffer";
import { useEffect, useRef, useState } from "react";
import {
  ImageBackground,
  PermissionsAndroid,
  Platform,
  StyleSheet,
  Text,
  View,
} from "react-native";
import { BleManager, Device } from "react-native-ble-plx";
import { ref, onValue, off } from "firebase/database";
import { database } from "./firebaseConfig";

const background = require("../assets/images/background1.png");

// UUIDs from your ESP32
const AQI_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const DATA_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

const DEVICE_NAME = "GOOD1-Meter";

const AQI = () => {
  const [temp, setTemp] = useState(0);
  const [humidity, setHumidity] = useState(0);
  const [pm25, setPm25] = useState(0);
  const [pm10, setPm10] = useState(0);
  const [aqi, setAqi] = useState(0);
  const [dataSource, setDataSource] = useState<"BLE" | "FIREBASE">("BLE");

  const managerRef = useRef<BleManager | null>(null);
  if (!managerRef.current) {
    managerRef.current = new BleManager();
  }
  const manager = managerRef.current;

  useEffect(() => {
    let stateSubscription: any;
    let connectedDevice: Device | null = null;
    let bleTimeout: NodeJS.Timeout;
    let firebaseRef: any = null;

    const requestPermissions = async () => {
      if (Platform.OS === "android") {
        const result = await PermissionsAndroid.requestMultiple([
          PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
          PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
          PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
        ]);

        return Object.values(result).every(
          status => status === PermissionsAndroid.RESULTS.GRANTED
        );
      }
      return true;
    };

    const startFirebaseMode = () => {
      console.log("Switching to Firebase mode");
      setDataSource("FIREBASE");

      firebaseRef = ref(database, "aqiData");

      onValue(firebaseRef, (snapshot) => {
        const data = snapshot.val();
        if (!data) return;

        setTemp(data.temperature ?? 0);
        setHumidity(data.humidity ?? 0);
        setPm25(data.pm25 ?? 0);
        setPm10(data.pm10 ?? 0);
        setAqi(data.aqi ?? 0);
      });
    };

    const scanAndConnect = () => {
      manager.startDeviceScan(null, null, (error, device) => {
        if (error) {
          console.log("Scan error:", error);
          startFirebaseMode();
          return;
        }

        if (
          device &&
          (device.name === DEVICE_NAME ||
            device.localName === DEVICE_NAME)
        ) {
          console.log("BLE Device Found");
          manager.stopDeviceScan();
          clearTimeout(bleTimeout);
          connectToDevice(device);
        }
      });

      // Fallback after 7 seconds
      bleTimeout = setTimeout(() => {
        console.log("BLE not found. Falling back.");
        manager.stopDeviceScan();
        startFirebaseMode();
      }, 7000);
    };

    const connectToDevice = async (device: Device) => {
      try {
        connectedDevice = await device.connect();
        await connectedDevice.discoverAllServicesAndCharacteristics();

        setDataSource("BLE");
        console.log("Connected to BLE");

        connectedDevice.monitorCharacteristicForService(
          AQI_SERVICE_UUID,
          DATA_CHAR_UUID,
          (error, characteristic) => {
            if (error) {
              console.log("Monitor error:", error);
              startFirebaseMode();
              return;
            }

            if (!characteristic?.value) return;

            const decodedString = Buffer.from(
              characteristic.value,
              "base64"
            ).toString("utf-8");

            const values = decodedString.split(",");

            if (values.length >= 3) {
              setTemp(parseFloat(values[0]));
              setHumidity(parseFloat(values[1]));
              setAqi(parseFloat(values[2]));
            }
          }
        );
      } catch (err) {
        console.log("BLE connection error:", err);
        startFirebaseMode();
      }
    };

    const init = async () => {
      const granted = await requestPermissions();
      if (!granted) {
        console.log("BLE permissions denied");
        startFirebaseMode();
        return;
      }

      stateSubscription = manager.onStateChange(
        (state) => {
          if (state === "PoweredOn") {
            scanAndConnect();
            stateSubscription.remove();
          }
        },
        true
      );
    };

    init();

    return () => {
      console.log("Cleaning up");

      clearTimeout(bleTimeout);
      manager.stopDeviceScan();
      connectedDevice?.cancelConnection();
      stateSubscription?.remove();
      manager.destroy();

      if (firebaseRef) {
        off(firebaseRef);
      }
    };
  }, []);

  const styles = StyleSheet.create({
    container: { flex: 1, justifyContent: "center" },
    tempWrapper: { flex: 1, justifyContent: "center", alignItems: "center" },
    text: {
      fontSize: 75,
      fontWeight: "100",
      color: "white",
    },
    data: {
      flex: 1,
      justifyContent: "center",
      alignItems: "center",
      width: "100%",
    },
    dataWrapper: {
      backgroundColor: "rgba(255,255,255,0.2)",
      flexDirection: "row",
      height: "40%",
      justifyContent: "center",
      alignItems: "center",
      width: "50%",
      borderRadius: 20,
      borderWidth: 1,
      borderColor: "white",
      marginVertical: 7,
    },
    dataText: {
      fontSize: 28,
      fontWeight: "200",
      color: "white",
      textAlign: "center",
    },
    title: {
      fontSize: 20,
      fontWeight: "bold",
      color: "white",
      textAlign: "center",
    },
    Wrapper: {
      width: "90%",
      height: "60%",
      flexWrap: "wrap",
    },
    source: {
      position: "absolute",
      top: 40,
      alignSelf: "center",
      color: "white",
      fontSize: 14,
    },
  });

  return (
    <ImageBackground source={background} style={styles.container}>
      <Text style={styles.source}>Source: {dataSource}</Text>

      <View style={styles.tempWrapper}>
        <Text style={styles.text}>AQI: {aqi}</Text>
      </View>

      <View style={styles.data}>
        <View style={styles.Wrapper}>
          <View style={styles.dataWrapper}>
            <Text style={styles.dataText}>
              {pm25} μg/m³{"\n"}PM 2.5
            </Text>
          </View>

          <View style={styles.dataWrapper}>
            <Text style={styles.dataText}>
              {temp}°C{"\n"}Temperature
            </Text>
          </View>

          <View style={styles.dataWrapper}>
            <Text style={styles.dataText}>
              {pm10} μg/m³{"\n"}PM 10
            </Text>
          </View>

          <View style={styles.dataWrapper}>
            <Text style={styles.dataText}>
              {humidity}%{"\n"}Humidity
            </Text>
          </View>
        </View>
      </View>
    </ImageBackground>
  );
};

export default AQI;