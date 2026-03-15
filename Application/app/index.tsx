import { StatusBar } from "expo-status-bar";
import { StyleSheet, View } from "react-native";
import Weather from "../components/screen";

export default function App() {
  return (
    <View style={styles.container}>
      <Weather />

      <StatusBar style="auto" />
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
});