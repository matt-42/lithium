import { ListItem } from "@material-ui/core";
import Container from "@material-ui/core/Container";
import List from "@material-ui/core/List/List";
import Paper from "@material-ui/core/Paper";
import Typography from "@material-ui/core/Typography/Typography";
import hljs from 'highlight.js';
import "highlight.js/styles/vs2015.css";
import marked, { lexer } from "marked";
import React, { useEffect, useState } from "react";
import { useHistory, useParams } from "react-router-dom";
import { setLocation } from "./App";

const docUrls: { [s: string]: string } = {
  "getting-started": "https://raw.githubusercontent.com/matt-42/lithium/master/docs/getting_started.md",
  "sql": "https://raw.githubusercontent.com/matt-42/lithium/master/docs/sql.cc",
  "http-server": "https://raw.githubusercontent.com/matt-42/lithium/master/docs/http_server.cc",
  "json": "https://raw.githubusercontent.com/matt-42/lithium/master/docs/json.cc"
}

function formatUrl(s: string) {
  return s.replace(/[^a-zA-Z0-9]/g, "-").toLowerCase();
}

const docRenderer = {
  code(code: string, infostring: string, escaped: boolean) {
    return `<pre><code class="hljs cpp c++">${hljs.highlight("c++", code).value}</code></pre>`;
  },
  heading(text: string, level: number) {
    const escapedText = text.toLowerCase().replace(/[^\w]+/g, '-');
    return `
    <a name="${escapedText}" class="anchor" href="#${escapedText}" style="top: -90px; display: block;
    position: relative;
    top: -60px;
    visibility: hidden;">
    <h${level} class="MuiTypography-root MuiTypography-h${level}" style="margin-bottom: ${10 * (6 - level)}px; margin-top: ${10 * (6 - level)}px">
    <span class="header-link"></span>
    </a>
    ${text.toLowerCase()}
    </h${level}>`;
  },
  paragraph(src: string) {
    return `<p class="MuiTypography-root MuiTypography-body1">${src}</p>`
  }
};

function cppToMarkdown(code: string) {
  // Convert c++ to markdown:
  // replace c++ comment with markdown c++ code
  let marker = "__documentation_starts_here__";
  let markerpos = code.indexOf(marker)
  if (markerpos != -1)
    code = code.substring(markerpos + marker.length);
  code = code.replace('/*', '```\n');
  code = code.replace('*/', '```c++\n');
  code = '```c++\n' + code + '```\n';

  code = code.replace(/```c\+\+[\n ]*```/, '');
  return code;
}

interface SectionNode {
  text: string,
  depth: number,
  children: { [k: string]: SectionNode },
  parent: SectionNode | null
};

export const sectionUrl = (item: SectionNode) => {
  let url = "/";
  if (item.parent)
    return url + formatUrl(item.parent.text) + "#" + formatUrl(item.text);
  else
    return url + formatUrl(item.text);
}

export const sectionPath = (item: SectionNode) => {
  if (item.parent)
    return item.parent.text + " / " + item.text;
  else
    return item.text;
}

export type DocHierarchy = { [k: string]: SectionNode };
export type DocIndexEntry = { text: string, section: SectionNode };
export type DocIndex = DocIndexEntry[];

export async function indexDocumentation()
  : Promise<[DocHierarchy, DocIndex]> {

  let searchIndex: DocIndex = [];
  let hierarchy: DocHierarchy = {};

  for (let url of Object.values(docUrls)) {
    let content: string = await fetch(url).then(r => r.text());

    let items: any[] = lexer(url.split('.').pop() == "md" ? content : cppToMarkdown(content));

    let parents: (SectionNode | null)[] = [];
    for (let item of items) {
      if (item.type === "heading") {

        let itempos = item.depth - 1;
        let parentpos = item.depth - 2;

        if (item.depth > 1) {
          // find parent.
          while (parentpos > 0 && !parents[parentpos]) parentpos--;

          let parent = parents[parentpos] as SectionNode;
          if (!parent)
          {
            console.log(parents);
            console.log(item);
          }
          // add item as child to parent.
          parent.children[item.text] = { text: item.text, depth: item.depth, children: {}, parent };

          // add item to the parent array.
          // while (parents.length < item.depth) parents.push(null);
          parents.length = itempos + 1;
          console.log("new length: ", parents.length);
          parents[itempos] = parent.children[item.text];
        }
        else {
          hierarchy[item.text] = { text: item.text, depth: item.depth, children: {}, parent: null };
          parents.length = itempos + 1;
          console.log("new length: ", parents.length);
          parents[0] = hierarchy[item.text];
        }

        // index item
        searchIndex.push({ text: item.text || "", section: parents[itempos] as SectionNode });
      }
      else {
        // index non headings nodes.
        if (parents.length)
          searchIndex.push({ text: item.text || "", section: parents[parents.length - 1] as SectionNode });
      }
    }
  }
  return [hierarchy, searchIndex];
}

export const documentationIndex = indexDocumentation();

async function generateDocumentation(doc_url: string) {
  marked.use({ renderer: docRenderer as any });

  // Fecth the c++ code.
  let code: string = await fetch(doc_url).then(r => r.text());
  // Remove doc preambule.

  return marked(doc_url.split('.').pop() == "md" ? code : cppToMarkdown(code));
}

export const Documentation = () => {

  const history = useHistory();
  let { sectionId } = useParams<{ sectionId: string }>();

  const [content, setContent] = useState("")
  const [menu, setMenu] = useState<any>(null)

  function makeMenu(hierarchy: DocHierarchy) {

    if (!hierarchy) return <></>;

    return <List disablePadding>
      {Object.values(hierarchy).map((item: SectionNode) => <>
        <ListItem key={item.text} button
          component="a"
          href={sectionUrl(item)}  
          // ContainerProps={{ onClick: (e) => e.preventDefault() }}
          onClick={(e:any) => {
            e.preventDefault()
            setLocation(sectionUrl(item), history);
          }}//"return false;"        
          style={{ paddingLeft: `${10 * item.depth}px` }}>
          {
            !item.parent ? <span style={{fontFamily: "Major Mono Display"}}>{item.text.toLowerCase()}</span>
            : <Typography>{item.text}</Typography>          
          }
        </ListItem>
        {makeMenu(item.children)}
      </>)}
    </List>
  }


  useEffect(() => {
    (async () => {
      let menu = makeMenu((await documentationIndex)[0]);
      console.log(menu);
      setMenu(menu);
      if (!docUrls[sectionId])
        setContent(sectionId + ": Page not found");
      else
        await generateDocumentation(docUrls[sectionId]).then(c => setContent(c));
      let prevHash = window.location.hash;
      window.location.hash = "";
      window.location.hash = prevHash;
    })();

  }, [sectionId]);


  return <div>
<Container style={{paddingLeft: "240px", position: "relative", paddingTop: "100px"}}>

      <div style={{ position: "fixed", width: "200px", top: "100px", marginLeft: "-240px", height: "100%", overflow: "scroll-y" }}>
        {menu}
      </div>
      <Paper style={{ flexGrow: 1, textAlign: "left", padding: "20px", }}>
        <div dangerouslySetInnerHTML={{ __html: content }}>
        </div>
      </Paper>
</Container>

  </div>

}
