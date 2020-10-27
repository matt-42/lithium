import { Container } from '@material-ui/core';
import AppBar from '@material-ui/core/AppBar';
import Icon from '@material-ui/core/Icon/Icon';
import IconButton from '@material-ui/core/IconButton';
import Link from '@material-ui/core/Link/Link';
import { createStyles, makeStyles, Theme } from '@material-ui/core/styles';
import useTheme from '@material-ui/core/styles/useTheme';
import Toolbar from '@material-ui/core/Toolbar';
import Typography from '@material-ui/core/Typography';
import useMediaQuery from '@material-ui/core/useMediaQuery/useMediaQuery';
import GitHub from '@material-ui/icons/GitHub';
import React from 'react';
import brushed_bg from "./images/brushed.jpg";
import brushed_bg_white from "./images/brushed_white.jpg";
import { Search2 } from './Search2';

const useStyles = makeStyles((theme: Theme) => {

  let dark = theme.palette.type === 'dark';
  return createStyles({
    appbar: {
      backgroundImage: `url("${  dark ? brushed_bg : brushed_bg_white }")`,
      backgroundColor: dark ? "black" : "white"
    },
    grow: {
      flexGrow: 1,
    },
    menuButton: {
      marginRight: theme.spacing(2),
    },
    title: {
      color: theme.palette.text.primary,
      display: 'none',
      fontFamily: "Major Mono Display",
      fontSize: "25px",
      [theme.breakpoints.up('sm')]: {
        display: 'block',
      },
      textDecoration: "none"
    },

    inputRoot: {
      color: 'inherit',
    },
    inputInput: {
      padding: theme.spacing(1, 1, 1, 0),
      // vertical padding + font size from searchIcon
      paddingLeft: `calc(1em + ${theme.spacing(4)}px)`,
      transition: theme.transitions.create('width'),
      width: '100%',
      [theme.breakpoints.up('md')]: {
        width: '20ch',
      },
    },
    sectionDesktop: {
      display: 'none',
      [theme.breakpoints.up('md')]: {
        display: 'flex',
      },
    },
    sectionMobile: {
      display: 'flex',
      [theme.breakpoints.up('md')]: {
        display: 'none',
      },
    },
  })

}
);


export default function PrimarySearchAppBar(props : { switchDarkMode: () => void }) {

  const screen450 = useMediaQuery('(min-width:450px)');
  const theme = useTheme();
  const classes = useStyles();
  
  return (
    <div className={classes.grow}>
      <AppBar position="fixed" className={classes.appbar}>
        <Container>
          <Toolbar>
            <a hidden={!screen450} href={process.env.PUBLIC_URL}>
              <img alt="Lithium logo" src="https://raw.githubusercontent.com/matt-42/lithium/master/images/lithium_logo_white.png" width="40" 
                  style={{ paddingRight: "15px", filter: `invert(${theme.palette.type === "dark" ? "0" : '100'})` }} />
            </a>
            <Typography className={classes.title} variant="h6" noWrap>
              <Link className={classes.title}  underline="none" href="#">lithium</Link>
          </Typography>
            <div className={classes.grow} />
            <div>
              <Search2></Search2>
          </div>
            <div>
              <IconButton href="https://github.com/matt-42/lithium" target="_blank" aria-label="Github" >
                  <GitHub />
                </IconButton>
                <IconButton onClick={props.switchDarkMode}>
                  <Icon>invert_colors</Icon>
                </IconButton>
            </div>
          </Toolbar>
        </Container>
      </AppBar>
    </div>
  );
}
