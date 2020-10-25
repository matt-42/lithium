import { createMuiTheme, createStyles, makeStyles, Theme, ThemeProvider } from '@material-ui/core/styles';
import Typography from '@material-ui/core/Typography';
import React, { useEffect, useState } from 'react';
import './App.css';
import PrimarySearchAppBar from './AppBar';
import { Documentation } from './Documentation';
import { HomePage } from './HomePage';
import brushed_bg from  './images/brushed.jpg'
import brushed_white_bg from  './images/brushed_white.jpg'

const darkTheme = createMuiTheme({
  palette: {
    type: 'dark',
    primary: {
      main: '#f5f5f5',
      dark: '#414141'
    },
    secondary: {
      main: '#f5f5f5',
      dark: '#414141'
    },
    // background: {
    //   paper: "rgba(255,255,255,0.5)",      
    //   default: "rgba(255,255,255,0.5)"
    // }
    background: { paper: "#151515" },
  },

  typography: {
    h1: {
      fontFamily: "Major Mono Display",
      fontSize: "50px"
    },
    h2: {
      fontFamily: "Major Mono Display",
      fontSize: "30px"
    },
    h3: {
      fontFamily: "Major Mono Display",
      fontSize: "20px"
    },
    h4: {
      fontFamily: "Major Mono Display",
      fontSize: "15px",
      
    },
  },


});

// const useStyles = makeStyles((theme: Theme) =>
//   createStyles({
//     // body : {
//     //   backgroundImage: `url("${theme.palette.type == "dark" ? brushed_bg : brushed_white_bg}")`
//     // }
//   }));

function AppContent() {

  // const styles = useStyles();
  const [windowHash, setwindowHash] = useState(window.location.hash);
  
  useEffect(() => {
    window.onhashchange = () => setwindowHash(window.location.hash);
  }, []);

  return windowHash == "" ? <HomePage /> : <Documentation hash={windowHash}/>

}
function App() {

  return (
    <ThemeProvider theme={darkTheme}>
      {/* <div className={styles.body}> */}
        <PrimarySearchAppBar />
        <AppContent/>    
      {/* </div> */}
    </ThemeProvider>
  );
}

export default App;
