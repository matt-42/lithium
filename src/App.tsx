import React from 'react';
import logo from './logo.svg';
import './App.css';
import PrimarySearchAppBar from './AppBar';
import { createMuiTheme, ThemeProvider } from '@material-ui/core/styles';
import { Container, Box, Button, Typography } from '@material-ui/core';
import { HomePage } from './HomePage';

const darkTheme = createMuiTheme({
  palette: {
    type: 'dark',
    primary: {
      main: '#f5f5f5',
      // dark: '#414141'
    }
  },
});

function App() {
  return (
    <ThemeProvider theme={darkTheme}>
      <PrimarySearchAppBar />
      <HomePage></HomePage>
      <div>

        <Container className="App" style={{ display: "flex" }}>
{/* 
          <div style={{ flexGrow: 1 }}>
            <Button variant="contained" color="primary">Test</Button>
          Content
          <Typography>Content</Typography>
        </div>
          <div style={{ flexBasis: "300px" }}>
            Menu
        </div> */}
        </Container >
      </div>
    </ThemeProvider>
  );
}

export default App;
