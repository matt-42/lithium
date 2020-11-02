import { useMediaQuery } from '@material-ui/core';
import { createMuiTheme, createStyles, makeStyles, Theme, ThemeProvider, useTheme } from '@material-ui/core/styles';
import React, { useCallback, useEffect, useState } from 'react';
import './App.css';
import PrimarySearchAppBar from './AppBar';
import { Documentation } from './Documentation';
import { HomePage } from './HomePage';
import brushed_bg from "./images/brushed.jpg";
import brushed_bg_white from "./images/brushed_white.jpg";


const darkTheme = createMuiTheme({
  palette: {
    type: 'dark',
    primary: {
      main: '#e8e8e8',
      dark: '#414141'
    },
    secondary: {
      main: '#e8e8e8',
      dark: '#414141'
    },
    background: {
      paper: "#151515",      
      default: "#000000"
    }
    // background: { paper: "#151515" },
  },

  typography: {
    
    body1: {
      color: "#e8e8e8",
    },
    h1: {
      fontFamily: "Major Mono Display",
      fontSize: "50px",
      color: "#e8e8e8"
    },
    h2: {
      fontFamily: "Major Mono Display",
      fontSize: "30px",
      color: "#e8e8e8",
    },
    h3: {
      fontFamily: "Major Mono Display",
      fontSize: "20px",
      color: "#e8e8e8"
    },
    h4: {
      fontFamily: "Major Mono Display",
      fontSize: "15px",
      color: "#e8e8e8"
    },
  },


});

const lightTheme = createMuiTheme({
  palette: {
    type: 'light',
    secondary: {
      main: "#555555"
    },
    primary: {
      main: "#555555"
    },
    background: {
      paper: "#f5f5f5",      
      default: "#f5f5f5"
    }
  },

  typography: {
    body1: {
      color: "#151515",
    },
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
      fontSize: "15px"

    },
  },

});

const useStyles = makeStyles((theme: Theme) => {

  let dark = theme.palette.type === 'dark';

  return createStyles({
    body: {

      // backgroundImage: `url("${  dark ? brushed_bg : brushed_bg_white }")`,
      backgroundColor: dark ? "#202020" : "#e8e8e8"

    }
  });
});

declare global {
  var gtag : any;
};

function AppContent() {

  const [windowHash, setwindowHash] = useState(window.location.hash);

  useEffect(() => {
    window.onhashchange = () => {
      setwindowHash(window.location.hash);
      gtag('event', 'page_view', {
        page_path: window.location.pathname + window.location.search + window.location.hash,
        send_to: 'G-KPG7NGCZH9'
      });
    }
  }, []);

  return windowHash === "" ? <HomePage /> : <Documentation hash={windowHash} />

}
function Body(props : { switchDarkMode: () => void }) {
  let styles = useStyles();

  let theme = useTheme();
  
  useEffect(() => {
    let codeTheme = theme.palette.type === "light" ? "//cdnjs.cloudflare.com/ajax/libs/highlight.js/10.1.2/styles/default.min.css" :
      "//cdnjs.cloudflare.com/ajax/libs/highlight.js/10.1.2/styles/vs2015.min.css";

    document.querySelector("link[id='highlightJSTheme']")?.setAttribute("href", codeTheme);
  }, [theme]);

  return (
    <div className={styles.body}>
      <PrimarySearchAppBar switchDarkMode={props.switchDarkMode}  />
      <AppContent />
    </div>
  );

}


function App() {

  const cssPreference = useMediaQuery('(prefers-color-scheme: dark)')
  const [darkMode, setDarkMode] = useState(false);

  useEffect(() => {
    let storage = window.localStorage.getItem("darkMode");
    if (storage === "true") return setDarkMode(true);
    else if (storage === null) return setDarkMode(cssPreference);
    else setDarkMode(false);
  }, [cssPreference]);

  const theme = React.useMemo(() => darkMode ? darkTheme : lightTheme, [darkMode]);

  const switchDarkMode = useCallback(() => {
    // setDarkMode(!darkMode); material ui theme switch does not work, need to reload the page.
    window.localStorage.setItem("darkMode", `${!darkMode}`);
    window.location.reload(); 

  }, [darkMode, setDarkMode]);

  return (
    <ThemeProvider theme={theme}>
      <Body switchDarkMode={switchDarkMode} />
    </ThemeProvider>
  );
}

export default App;
