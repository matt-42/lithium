import { createMuiTheme, ThemeProvider } from '@material-ui/core/styles';
import React, { useEffect, useState } from 'react';
import './App.css';
import PrimarySearchAppBar from './AppBar';
import { Documentation } from './Documentation';
import { HomePage } from './HomePage';

const darkTheme = createMuiTheme({
  palette: {
    type: 'dark',
    primary: {
      main: '#f5f5f5',
      // dark: '#414141'
    }
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
    }
  },
  
});

function App() {
  const [windowHash, setwindowHash] = useState(window.location.hash);

  useEffect(() => {
    window.onhashchange = () => setwindowHash(window.location.hash);
  }, []);
  
  return (

      <ThemeProvider theme={darkTheme}>
        <PrimarySearchAppBar />
        {
          windowHash == "" ? <HomePage/> : <Documentation hash={windowHash}/>
        }
        
    
      </ThemeProvider>
  );
}

export default App;
