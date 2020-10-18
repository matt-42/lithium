import React from 'react';
import logo from './logo.svg';
import './App.css';
import PrimarySearchAppBar from './AppBar';
import { createMuiTheme, ThemeProvider } from '@material-ui/core/styles';
import { Container, Box, Button, Typography } from '@material-ui/core';
import { HomePage } from './HomePage';
import { Documentation } from './Documentation';

import {
  BrowserRouter as Router,
  Switch,
  Route,
  Link
} from "react-router-dom";
import { createBrowserHistory } from 'history';


// export const history = createBrowserHistory({
//   basename: process.env.PUBLIC_URL
// });

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
    }
  }
});

function App() {
  return (
    <Router basename={process.env.PUBLIC_URL}>

      <ThemeProvider theme={darkTheme}>
        <PrimarySearchAppBar />

        <Switch>
          <Route path="/:sectionId">
            <Documentation />
          </Route>
          <Route path="/">
            <HomePage></HomePage>
          </Route>
        </Switch>
        
    
      </ThemeProvider>
    </Router>
  );
}

export default App;
