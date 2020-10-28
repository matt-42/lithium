import Container from "@material-ui/core/Container"
import Link from "@material-ui/core/Link"
import Typography from "@material-ui/core/Typography"
import Button from "@material-ui/core/Button"
import Icon from "@material-ui/core/Icon"
import React, { useEffect } from "react"
import { HLcpp, HLsh, InlineCode } from "./HL"
import Paper from "@material-ui/core/Paper"
import ListItem from "@material-ui/core/ListItem"
import List from "@material-ui/core/List"
import ListItemText from "@material-ui/core/ListItemText"
import useMediaQuery from "@material-ui/core/useMediaQuery"
import { Footer } from "./Footer"
import { useTheme } from "@material-ui/core/styles"
  
export const HomePage = () => {
  const theme = useTheme();
  const screen700 = useMediaQuery('(min-width:700px)');

  useEffect(() => {
    const script = document.createElement("script");
    script.src = "https://platform.twitter.com/widgets.js";
    document.getElementsByClassName("twitter-share-button")[0].appendChild(script);
  }, []);

  return <Container>
    <div style={{ display: "flex", alignItems: "center", 
                  flexDirection: screen700 ? "row":"column",
                  justifyContent: "center", paddingTop: "100px" }}>

      <img src="https://raw.githubusercontent.com/matt-42/lithium/master/images/lithium_logo_white.png" 
          width="200" alt="Lithium. Build high performance C++ HTTP servers without being a C++ expert."
          style={{marginRight: "30px", filter: `invert(${theme.palette.type === "dark" ? "0" : '100'})` }} />

      <div >
        <Typography variant="h1">lithium</Typography>
        <Typography style={{ fontSize: "22px" }} >
          Build high performance C++ HTTP servers without being a C++ expert.
      </Typography>
        <div style={{ height: "20px" }}></div>
        <div style={{ display: "flex", alignItems: "center", justifyContent: "flex-start", }}>    
        <Button variant="outlined"  style={{marginRight: "45px"}} href={"#getting-started"} >Get Started</Button>

        <iframe src="https://ghbtns.com/github-btn.html?user=matt-42&repo=lithium&type=star&count=true&size=large" frameBorder="0" scrolling="0" width="170" height="30" title="GitHub"></iframe>
        <a href="https://twitter.com/share" className="twitter-share-button" style={{float: "left", marginLeft: "-37px", paddingTop: "10px"}} data-size="large">Tweet</a>
    </div>

      </div>

    </div>



    <div style={{ display: "flex", flexDirection: "column",  alignItems: "center", paddingTop: "100px"}}>
      <Paper style={{ flex: 1,marginBottom: "20px", padding: "20px", width: screen700 ? "700px" : "calc(100% - 20px)",  }}>
        <div style={{ display: "flex", alignItems: "center" }}>
          <Icon>get_app</Icon>
          <div style={{flexBasis: "10px"}}></div>
          <Typography variant="h6"> Installation</Typography>
        </div>
        <div style={{ height: "20px" }}></div>

<Typography>
        The simplest way to use Lithium is to download the cli in your path. It compiles and runs the server's code
        inside a docker container. The only requirements are Docker and Python>=3.6.
</Typography>

        <HLsh>
{
`$ wget https://raw.githubusercontent.com/matt-42/lithium/master/cli/li
$ chmod +x ./li`}</HLsh>
<Typography>
        You can also instal Lithium locally. <Link href="#getting-started"><u>Check the install guide for more info</u></Link>. 
</Typography>

      </Paper>

      <Paper style={{ flex: 1, padding: "20px", width: screen700 ? "700px" : "calc(100% - 20px)", }}>

      <div style={{ display: "flex", alignItems: "center" }}>
          <Icon>api</Icon>
          <div style={{flexBasis: "10px"}}></div>
          <Typography variant="h6">Hello World</Typography>
        </div>
        <div style={{ height: "20px" }}></div>

        <Typography>
          Write your first <InlineCode>hello_world</InlineCode> Lithium server.
          </Typography>
        <HLcpp>
          {
`// main.cc 
#include <lithium_http_server.hh>
            
int main() {
  http_api my_api;
              
  my_api.get("/hello_world") = 
  [&](http_request& request, http_response& response) {
    response.write("hello world.");
  };
  http_serve(my_api, 8080);
}
`}
        </HLcpp>

        <Typography>
        And run it:
        </Typography>

        <HLsh>{
          `$ li run ./main.cc`}</HLsh>

        <Typography>
          Behind the scene, <InlineCode>li</InlineCode> will build a lithium docker image, compile and run the server.
        </Typography>

        <p style={{textAlign:"right"}}>
        <Button variant="outlined" href="#getting-started">Read the docs</Button>
        </p>

      </Paper>
      <div style={{ height: "50px" }}></div>
      <div style={{ 
        borderTop: "1px solid #4a4a4a",
        borderBottom: "1px solid #222222",
       width: "70%" }}></div>
      <div style={{ height: "50px" }}></div>

      <Typography variant="h2" component="h3" style={{fontFamily: "Major Mono Display"}}>lithium's sponsors</Typography>

      <List>
        <ListItem button component="a" target="_blank" href="https://github.com/Burnett01" >
          <ListItemText primary="Steven Agyekum (Burnett01)" />
        </ListItem>
      </List>
  
      <Typography>If you find this project helpful and want to support it, give a star to lithium or buy me a coffee!</Typography>

      <Link  target="_blank"  href="https://github.com/sponsors/matt-42"><u>More info on my github sponsor page <Icon style={{color: "pink"}}>favorite_border</Icon></u></Link> 

      <br/>
      <Typography variant="h2" component="h3" style={{fontFamily: "Major Mono Display"}}>thanks</Typography>
      <Typography>Big thanks to Yvan Darmet for the logo and the design tips.</Typography>
    </div>

    <Footer/>
  </Container>
} 