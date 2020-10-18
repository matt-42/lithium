import Container from "@material-ui/core/Container"
import React, { useEffect, useState } from "react"
import marked, { lexer } from "marked"
import "highlight.js/styles/vs2015.css"
import Paper from "@material-ui/core/Paper";
import { HL } from "./HL";
import hljs from 'highlight.js'
import { ListItem, Typography } from "@material-ui/core";
import List from "@material-ui/core/List/List";
import { useParams, useRouteMatch } from "react-router-dom";
import Drawer from "@material-ui/core/Drawer/Drawer";

const docUrls : { [s: string] : string } = {
  "sql": "https://raw.githubusercontent.com/matt-42/lithium/master/docs/sql.cc"
}


function formatUrl(s : string) {
  return s.replace(/[^a-zA-Z0-9]/g, "-").toLowerCase();
}


const docRenderer = {
  code(code: string, infostring: string, escaped: boolean) {
    return `<pre><code class="hljs cpp c++">${hljs.highlight("c++", code).value}</code></pre>`;
    // return <HL language="${infostring}">${code}</HL>
  },
  // link(src: string) {
  //   return `<Link></Link>`
  // }
  heading(text: string, level: number) {
    const escapedText = text.toLowerCase().replace(/[^\w]+/g, '-');

    return `
    <h${level} class="MuiTypography-root MuiTypography-h${level}" style="margin-bottom: ${10 * (6 - level)}px; margin-top: ${10 * (6 - level)}px">
    <a name="${escapedText}" class="anchor" href="#${escapedText}">
    <span class="header-link"></span>
    </a>
    ${text.toLowerCase()}
    </h${level}>`;
  },
  paragraph(src: string) {
    return `<p class="MuiTypography-root MuiTypography-body1">${src}</p>`
  }
};


const docIndexer = {
  heading(text: string, level: number) {
    console.log("heading ", text, level);
    return text;
  }

}

function cppToMarkdown(code: string) {
  // Convert c++ to markdown:
  // replace c++ comment with markdown c++ code
  code = code.substring(code.indexOf("__documentation_starts_here__") + "__documentation_starts_here__".length);
  // code = code.replace(/.*__documentation_starts_here__/, "");
  code = code.replace('/*', '```\n');
  code = code.replace('*/', '```c++\n');
  code = '```c++\n' + code + '```\n';

  code = code.replace(/```c\+\+[\n ]*```/, '');
  return code;
}

async function indexDocumentation() {
  let hierarchy: { [k: string]: any } = {};
  // marked.use({ renderer: docIndexer as any });
  for (let url of Object.values(docUrls)) {
    let cpp: string = await fetch(url).then(r => r.text());

    let items: any[] = lexer(cppToMarkdown(cpp));
    let parents: any[] = [];
    for (let item of items) {
      if (item.type === "heading") {

        let itempos = item.depth - 1;
        let parentpos = item.depth - 2;

        if (item.depth > 1) {
          // find parent.
          while (parentpos > 0 && !parents[parentpos]) parentpos--;

          let parent = parents[parentpos];
          // add item as child to parent.
          parent.children[item.text] = { text: item.text, depth: item.depth, children: {}, parent };

          // add item to the parent array.
          while (parents.length < item.depth) parents.push(null);
          parents[itempos] = parent.children[item.text];
        }
        else {
          hierarchy[item.text] = { text: item.text, depth: item.depth, children: {}, parent: null };
          parents[0] = hierarchy[item.text];
        }




        // console.log(item.text, item.depth);
      }
    }
  }
  return hierarchy;
  console.log(hierarchy);
}

async function generateDocumentation(cpp_url: string) {
  marked.use({ renderer: docRenderer as any });

  // Fecth the c++ code.
  let code: string = await fetch(cpp_url).then(r => r.text());
  // Remove doc preambule.

  return marked(cppToMarkdown(code));
}

export const Documentation = () => {

  let { sectionId } = useParams<{sectionId:string}>();
  
  const [content, setContent] = useState("")
  const [menu, setMenu] = useState<any>(null)

  function makeMenu(hierarchy: any) {

    if (!hierarchy) return <></>;

    const itemUrl = (item : any) => {
      let url = process.env.PUBLIC_URL + "/";
      if (item.parent)
        return url + formatUrl(item.parent.text) + "#" + formatUrl(item.text);
    } 
    return <List disablePadding>
      {Object.values(hierarchy).map((item: any) => <>
        <ListItem key={item.text} button 
          component="a"
          href={itemUrl(item)}
        style={{paddingLeft: `${10*item.depth}px`}}>
          {item.text}
        </ListItem>
        {makeMenu(item.children)}
      </>)}
    </List>
  }


  useEffect(() => {
    (async () => {
      let menu = makeMenu(await indexDocumentation());
      console.log(menu);
      setMenu(menu);
      await generateDocumentation(docUrls[sectionId]).then(c => setContent(c));
    })();

  }, [sectionId]);


  return <div>

    <Container className="App" style={{ display: "flex" }}>

      <Drawer anchor="left" variant="permanent" style={{ flexBasis: "300px", 
      
      marginRight: "20px", padding: "20px" }}>
        {menu}
      </Drawer>
      <Paper style={{ flexGrow: 1, textAlign: "left", padding: "20px", }}>
        <div dangerouslySetInnerHTML={{ __html: content }}>
        </div>
      </Paper>


    </Container >
  </div>

}
