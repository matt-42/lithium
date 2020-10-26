import { Container } from '@material-ui/core';
import AppBar from '@material-ui/core/AppBar';
import Badge from '@material-ui/core/Badge';
import IconButton from '@material-ui/core/IconButton';
import Link from '@material-ui/core/Link/Link';
import Menu from '@material-ui/core/Menu';
import MenuItem from '@material-ui/core/MenuItem';
import { createStyles, makeStyles, Theme } from '@material-ui/core/styles';
import useTheme from '@material-ui/core/styles/useTheme';
import Toolbar from '@material-ui/core/Toolbar';
import Typography from '@material-ui/core/Typography';
import AccountCircle from '@material-ui/icons/AccountCircle';
import GitHub from '@material-ui/icons/GitHub';
import MailIcon from '@material-ui/icons/Mail';
import MoreIcon from '@material-ui/icons/MoreVert';
import NotificationsIcon from '@material-ui/icons/Notifications';
import React from 'react';
import { Search2 } from './Search2';
import brushed_bg from "./images/brushed.jpg"
import brushed_bg_white from "./images/brushed_white.jpg"
import Icon from '@material-ui/core/Icon/Icon';
import useMediaQuery from '@material-ui/core/useMediaQuery/useMediaQuery';

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
  const [anchorEl, setAnchorEl] = React.useState<null | HTMLElement>(null);
  const [mobileMoreAnchorEl, setMobileMoreAnchorEl] = React.useState<null | HTMLElement>(null);

  const isMenuOpen = Boolean(anchorEl);
  const isMobileMenuOpen = Boolean(mobileMoreAnchorEl);

  const handleProfileMenuOpen = (event: React.MouseEvent<HTMLElement>) => {
    setAnchorEl(event.currentTarget);
  };

  const handleMobileMenuClose = () => {
    setMobileMoreAnchorEl(null);
  };

  const handleMenuClose = () => {
    setAnchorEl(null);
    handleMobileMenuClose();
  };

  const handleMobileMenuOpen = (event: React.MouseEvent<HTMLElement>) => {
    setMobileMoreAnchorEl(event.currentTarget);
  };

  const menuId = 'primary-search-account-menu';
  // const renderMenu = (
  //   <Menu
  //     anchorEl={anchorEl}
  //     anchorOrigin={{ vertical: 'top', horizontal: 'right' }}
  //     id={menuId}
  //     keepMounted
  //     transformOrigin={{ vertical: 'top', horizontal: 'right' }}
  //     open={isMenuOpen}
  //     onClose={handleMenuClose}
  //   >
  //     <MenuItem onClick={handleMenuClose}>Profile</MenuItem>
  //     <MenuItem onClick={handleMenuClose}>My account</MenuItem>
  //   </Menu>
  // );

  // const mobileMenuId = 'primary-search-account-menu-mobile';
  // const renderMobileMenu = (
  //   <Menu
  //     anchorEl={mobileMoreAnchorEl}
  //     anchorOrigin={{ vertical: 'top', horizontal: 'right' }}
  //     id={mobileMenuId}
  //     keepMounted
  //     transformOrigin={{ vertical: 'top', horizontal: 'right' }}
  //     open={isMobileMenuOpen}
  //     onClose={handleMobileMenuClose}
  //   >
  //     <MenuItem>
  //       <IconButton aria-label="show 4 new mails" color="inherit">
  //         <Badge badgeContent={4} color="secondary">
  //           <MailIcon />
  //         </Badge>
  //       </IconButton>
  //       <p>Messages</p>
  //     </MenuItem>
  //     <MenuItem>
  //       <IconButton aria-label="show 11 new notifications" color="inherit">
  //         <Badge badgeContent={11} color="secondary">
  //           <NotificationsIcon />
  //         </Badge>
  //       </IconButton>
  //       <p>Notifications</p>
  //     </MenuItem>
  //     <MenuItem onClick={handleProfileMenuOpen}>
  //       <IconButton
  //         aria-label="account of current user"
  //         aria-controls="primary-search-account-menu"
  //         aria-haspopup="true"
  //         color="inherit"
  //       >
  //         <AccountCircle />
  //       </IconButton>
  //       <p>Profile</p>
  //     </MenuItem>
  //   </Menu>
  // );

  return (
    <div className={classes.grow}>
      <AppBar position="fixed" className={classes.appbar}>
        <Container>
          <Toolbar>
            {/* <IconButton
            edge="start"
            className={classes.menuButton}
            color="inherit"
            aria-label="open drawer"
          >
            <MenuIcon />
          </IconButton> */}
            <a hidden={!screen450} href="/">
              <img alt="Lithium logo" src="https://raw.githubusercontent.com/matt-42/lithium/master/images/lithium_logo_white.png" width="40" 
                  style={{ paddingRight: "15px", filter: `invert(${theme.palette.type === "dark" ? "0" : '100'})` }} />
            </a>
            <Typography className={classes.title} variant="h6" noWrap>
              <Link className={classes.title}  underline="none" href="#">lithium</Link>
          </Typography>
            <div className={classes.grow} />
            {/* <TextField>xxxxx</TextField> */}
            <div>
              {/* <div className={classes.searchIcon}>
                <SearchIcon />
              </div> */}
              <Search2></Search2>

              {/* <InputBase
                placeholder="Search…"
                classes={{
                  root: classes.inputRoot,
                  input: classes.inputInput,
                }}
                inputProps={{ 'aria-label': 'search' }}
              /> */}
            </div>
            <div>
              <IconButton href="https://github.com/matt-42/lithium" target="_blank" aria-label="Github" >
                  <GitHub />
                </IconButton>
                <IconButton onClick={props.switchDarkMode}>
                  <Icon>invert_colors</Icon>
                </IconButton>
            </div>
            {/* <div className={classes.sectionMobile}>
              <IconButton
                aria-label="show more"
                aria-controls={mobileMenuId}
                aria-haspopup="true"
                onClick={handleMobileMenuOpen}
                color="inherit"
              >
                <MoreIcon />
              </IconButton>
            </div> */}
          </Toolbar>
        </Container>
      </AppBar>
      {/* {renderMobileMenu}
      {renderMenu} */}
    </div>
  );
}
