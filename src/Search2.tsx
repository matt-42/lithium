import { createStyles, makeStyles, TextField, Theme, Typography, useTheme } from "@material-ui/core";
import Icon from "@material-ui/core/Icon/Icon";
import InputAdornment from "@material-ui/core/InputAdornment/InputAdornment";
import useMediaQuery from "@material-ui/core/useMediaQuery/useMediaQuery";
import _ from "lodash";
import React, { useCallback, useEffect, useRef, useState } from "react";
import { DocIndex, DocIndexEntry, documentationIndex, sectionAnchor, sectionPath } from "./Documentation";
import brushed_bg from "./images/brushed.jpg";
import brushed_bg_white from "./images/brushed_white.jpg";
import hljs from 'highlight.js';
import marked from "marked";



function filterOptions(options: DocIndexEntry[], query: string): DocIndexEntry[] {

  let queryWords = query.split(" ").filter(s => s.length !== 0).map(s => s.toLowerCase());
  let selected: [DocIndexEntry, number][] = [];
  // return options.filter(entry => {
  //   for (let w of queryWords)
  //   // if (entry.text.toLowerCase().includes(w) || sectionPath(entry.section).toLowerCase().includes(w))
  //     if (entry.text.indexOf(w) !== -1)
  //       return true;
  //   return false;
  // });
  for (let entry of options) {
    let score = 0;
    for (let w of queryWords) {
      for (let content of entry.contents) {
        if (content.text?.toLowerCase().includes(w))
          score++;
      }

      if (entry.section.text.toLowerCase().includes(w))
        score += 10;
    }
    if (score > 0) selected.push([entry, score]);
  }
  selected = _.uniqBy(selected, (s) => s[0].section);
  selected = _.sortBy(selected, (s) => -s[1]);

  return selected.map(s => s[0]).slice(0, 20);
}


const useStyles = makeStyles((theme: Theme) => {

  return createStyles({
    searchResult: {
      "&:hover": { backgroundColor:  theme.palette.background.paper },
      borderLeft: "3px solid #999999",
    }
  });
});


function scrollIntoViewIfNeeded(target : HTMLElement) { 
  if (target.getBoundingClientRect().bottom > window.innerHeight) {
      target.scrollIntoView(false);
  }

  if (target.getBoundingClientRect().top < 0) {
      target.scrollIntoView();
  } 
}
export const SearchResult = (props: { option: DocIndexEntry, query: string, selected: boolean, onSelect: () => void }) => {

  const styles = useStyles();
  const divRef = useRef<HTMLDivElement>(null);
  useEffect(() => {
    if (props.selected && divRef.current)
      scrollIntoViewIfNeeded(divRef.current)
      // divRef.current?.scrollIntoView();
  },[props.selected, divRef])

  const screen700 = useMediaQuery('(min-width:700px)');
  let theme = useTheme();
  let { query, option, selected } = props;
  
  let queryWords = query.split(" ").filter(s => s.length !== 0);
  
  let queryWordsRegexp = queryWords.map(w => w.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')).join('|');
  
  let snippets: string[] = [];
  for (let content of option.contents) {
    
    const countNewLines = (str : string) => (str.match(/\n/g) || '').length + 1;

    let indices = queryWords.map(w => content.text.toLowerCase().indexOf(w.toLowerCase())).filter(i => i !== -1);
    if (content.type === "text") {
      let indicesGroups = indices.reduce<number[][]>((acc: number[][], cur: number, curIdx: number) => {
        if (curIdx === 0) return acc;
        if (countNewLines(content.text.substring((_.last(acc) as number[])[0], cur)) < 1) {
          _.last(acc)?.push(cur);
          return acc;
        }
        return [...acc, [cur]];
      }, [[indices[0]]]);
      //   if (curIdx === 0) return acc;
      //   if (cur - (_.last(acc) as number[])[0] < 100) {
      //     _.last(acc)?.push(cur);
      //     return acc;
      //   }
      //   else
      //     return [...acc, [cur]];
      // }, [[indices[0]]]);

      snippets.push(...indicesGroups.map(g => {
        let [min, max] = [g[0], g[g.length - 1]];
        if (max - min > 100) throw new Error();
        let [start, end] = [-50 + (max + min) / 2, 50 + (max + min) / 2];
        [start, end] = [Math.max(start, 0), Math.min(end, content.text.length)];
        let x = content.text.substring(0, start).lastIndexOf('\s')
        start = x === -1 ? start : x;
        x = content.text.substring(end).indexOf('\s');
        end = x === -1 ? end : end + x;

        while (content.text[start] !== '\n' && start > 0) start--;
        while (content.text[end] !== '\n' && end < content.text.length) end++;
        return marked(content.text.substring(start, end));
      }));
    }
    else if (content.type === "code") {

      let indicesGroups = indices.reduce<number[][]>((acc: number[][], cur: number, curIdx: number) => {
        if (curIdx === 0) return acc;
        if (countNewLines(content.text.substring((_.last(acc) as number[])[0], cur)) < 2) {
          _.last(acc)?.push(cur);
          return acc;
        }
        return [...acc, [cur]];
      }, [[indices[0]]]);

      snippets.push(...indicesGroups.map(g => {
        let [min, max] = [g[0], g[g.length - 1]];

        while (content.text[min] !== '\n' && min > 0) min--;
        while (content.text[max] !== '\n' && max < content.text.length) max++;

        let md = content.text.substring(min, max);
        const infostring = "c++";
        return `<pre><code class="hljs ${infostring}">${hljs.highlight(infostring, md).value}</code></pre>`;

      }));

    }
  }

  let snippet = snippets.join("");
  // let [snipetStart, snippetEnd] = [indices[0] - 50,indices[0] + 50];
  // if (indices.length > 1)
  // {
  //   indices.sort();
  //   let [min, max] = [indices[0], indices[indices.length - 1]];
  //   if (max - min < 100)
  //     [snipetStart, snippetEnd] = [-50 + (max + min) / 2, 50 + (max + min) / 2];

  //   // [snipetStart, snippetEnd] = [indices[0], indices[0] + 100]
  // } 
  // // Take 100 chars.
  // let snippet = option.text.substring(Math.max(snipetStart, 0), Math.min(snippetEnd, option.text.length));
  let highlight = (str: string) => {
    // for (let w of queryWords)
    // {
    str = str.replace(new RegExp(`(${queryWordsRegexp})`, 'gi'), `<span class="searchHighlight">$1</span>`);
    // }
    return str;
  };
  return <div
    ref={divRef}
    onClick={props.onSelect}
    className={styles.searchResult}
    key={sectionPath(option.section) + option.contents[0]?.text || ""}
    style={{
      cursor: "pointer",
      display: "flex",
      flexDirection: "column",
      borderLeft: (selected ? "3px" : "1px") +  " solid #999999",
      backgroundColor: selected ? theme.palette.background.paper : "auto",
      paddingLeft: "10px",
      textAlign: "left",
      marginTop: "10px",
      width: screen700 ? "700px" : "calc(100% - 20px)"
    }}>
    {
      true ? <div>
        <Typography variant="caption">{sectionPath(option.section)}</Typography>
      </div> : <></>
    }
    <div>
      <Typography variant="h6"><div dangerouslySetInnerHTML={{ __html: highlight(option.section.text) }} /></Typography>
    </div>
    <div>
      <div dangerouslySetInnerHTML={{ __html: highlight(snippet) }}></div>
    </div>
  </div>
}

export const Search2 = () => {
  const screen700 = useMediaQuery('(min-width:700px)');

  const theme = useTheme();
  const [index, setIndex] = useState<DocIndex | null>(null)
  const [query, setQuery] = useState("");
  const [selectedResultIndex, setSelectedResultIndex] = useState(0);
  const [results, setResults] = useState<DocIndexEntry[]>([]);

  // Focus on / or . key press.
  useEffect(() => {
    let l = (event: any) => {
      if (['/', '.'].includes(event.key))
        document.getElementById('doc_search_bar')?.focus();
    };
    document.addEventListener('keyup', l, false);
    return () => { document.removeEventListener('keyup', l); }
  }, []);

  const gotoResult = useCallback(
    (resultIdx: number) => {
      let selection = results[resultIdx];
      if (selection)
        window.location.hash = sectionAnchor(selection.section);
      setQuery("");
      document.getElementById('doc_search_bar')?.blur();
    },
    [results]);

  // Focus on / or . key press.
  useEffect(() => {
    let l = (event: any) => {
      if ('ArrowUp' === event.key)
        setSelectedResultIndex(Math.max(selectedResultIndex - 1, 0));
      if ('ArrowDown' === event.key)
        setSelectedResultIndex(Math.min(results.length - 1, selectedResultIndex + 1));
      if ('Enter' === event.key) {
        gotoResult(selectedResultIndex);
      }
      if ('Escape' === event.key) {
        setQuery("");
        document.getElementById('doc_search_bar')?.blur();
      }
    };
    document.addEventListener('keyup', l, false);
    return () => { document.removeEventListener('keyup', l); }
  }, [selectedResultIndex, results]);

  // Set index at mount.
  useEffect(() => {
    documentationIndex.then(index => {
      setIndex(index[1]);
    });
  }, []);

  // Select option on query change.
  useEffect(() => {
    if (index) {
      setResults(filterOptions(index, query));
      setSelectedResultIndex(0);
    }
  }, [query]);

  let searchInput = <TextField
    label={query.length ? "" : (screen700 ? "Search (Tap / or . to focus)" : "")}
    variant="outlined"
    size={query.length ? "medium" : "small"}
    // variant="filled"
    InputProps={{
      id: "doc_search_bar",
      style: {
        width: query.length ? (screen700 ? "600px" : "100%") : "200px",
        transitionProperty: "left top position width height",
        transitionDuration: "0.2s",
      },
      value: query,
      onChange: (e) => setQuery(e.target.value),
      startAdornment:
        <InputAdornment position="start">
          <Icon>search</Icon>
        </InputAdornment>
      ,
    }} />

  if (!index) return <></>

  let dark = theme.palette.type === 'dark';
  let searchStyle = {};
  if (query.length) searchStyle = {
    position: "fixed",
    display: "flex",
    flexDirection: "column",
    alignItems: "center",
    transitionProperty: "left top position width height",
    transitionDuration: "0.2s",
    top: "0px",
    left: "0px",
    width: "100%",
    height: "100%",
    zIndex: 2000,
    justifyContent: "center",
    backgroundImage: `url("${dark ? brushed_bg : brushed_bg_white}")`,
    backgroundColor: dark ? "black" : "white",
    color: theme.typography.body1.color
  };

  return <div
    style={searchStyle}>
    {searchInput}
    <div style={{ display: query.length ? "block" : "none", overflowY: "auto", height: "calc(100% - 150px)", marginTop: "10px" }}>
      {results.map((r, idx) => <SearchResult
        key={idx}
        option={r}
        query={query}
        selected={idx == selectedResultIndex}
        onSelect={() => gotoResult(idx)} />)}
    </div>
  </div>
}
