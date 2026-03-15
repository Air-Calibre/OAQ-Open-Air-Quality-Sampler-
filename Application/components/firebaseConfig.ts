// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";
import { getDatabase } from "firebase/database";
// TODO: Add SDKs for Firebase products that you want to use
// https://firebase.google.com/docs/web/setup#available-libraries

// Your web app's Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyDj9pPHQKQrly2IfoLPcbof4Pr6qcGnXus",
  authDomain: "aircal-a9be6.firebaseapp.com",
  databaseURL: "https://aircal-a9be6-default-rtdb.asia-southeast1.firebasedatabase.app",
  projectId: "aircal-a9be6",
  storageBucket: "aircal-a9be6.firebasestorage.app",
  messagingSenderId: "1035560514949",
  appId: "1:1035560514949:web:19abbccc5250d544f280ec"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);

export const database = getDatabase(app);