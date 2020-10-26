import { TextField, Typography, useTheme } from "@material-ui/core";
import Icon from "@material-ui/core/Icon/Icon";
import InputAdornment from "@material-ui/core/InputAdornment/InputAdornment";
import useMediaQuery from "@material-ui/core/useMediaQuery/useMediaQuery";
import _ from "lodash";
import React, { useCallback, useEffect, useState } from "react";
import { DocIndex, DocIndexEntry, documentationIndex, sectionAnchor, sectionPath } from "./Documentation";
import brushed_bg from "./images/brushed.jpg";
import brushed_bg_white from "./images/brushed_white.jpg";


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
      if (entry.text.toLowerCase().includes(w))
        score++;
      else if (entry.section.text.toLowerCase().includes(w))
        score += 2;
    }
    if (score > 0) selected.push([entry, score]);
  }
  selected = _.uniqBy(selected, (s) => s[0].section);
  selected = _.sortBy(selected, (s) => -s[1]);

  return selected.map(s => s[0]).slice(0, 20);
}

export const SearchResult = (props: { option: DocIndexEntry, query: string, selected: boolean, onSelect: () => void }) => {

  const screen700 = useMediaQuery('(min-width:700px)');
  let theme = useTheme();
  let { query, option, selected } = props;

  let queryWords = query.split(" ").filter(s => s.length !== 0);

  let queryWordsRegexp = queryWords.map(w => w.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')).join('|');

  let indices = queryWords.map(w => option.text.toLowerCase().indexOf(w.toLowerCase())).filter(i => i !== -1);
  let indicesGroups = indices.reduce<number[][]>((acc: number[][], cur: number, curIdx: number) => {
    if (curIdx === 0) return acc;
    if (cur - (_.last(acc) as number[])[0] < 100) {
      _.last(acc)?.push(cur);
      return acc;
    }
    else
      return [...acc, [cur]];
  }, [[indices[0]]]);

  let snippets: string[] = indicesGroups.map(g => {
    let [min, max] = [g[0], g[g.length - 1]];
    if (max - min > 100) throw new Error();
    let [start, end] = [-50 + (max + min) / 2, 50 + (max + min) / 2];
    [start, end] = [Math.max(start, 0), Math.min(end, option.text.length)];
    let x = option.text.substring(0, start).lastIndexOf('\s')
    start = x === -1 ? start : x;
    x = option.text.substring(end).indexOf('\s');
    end = x === -1 ? end : end + x;

    // while (option.text[start] !== ' ' && start > 0) start--;
    while (option.text[end] !== ' ' && end < option.text.length) end++;
    return option.text.substring(start, end);
  });

  let snippet = snippets.join(" [...] <br/> ");
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
    onClick={props.onSelect}
    key={sectionPath(option.section) + option.text}
    style={{
      cursor: "pointer",
      display: "flex",
      flexDirection: "column",
      borderLeft: "1px solid #999999",
      backgroundColor: selected ? theme.palette.background.paper : "unset",
      paddingLeft: "10px",
      textAlign: "left",
      marginTop: "10px",
      width: screen700 ? "700px" : "calc(100% - 20px)"
    }}>
    {
      option.section.parent ? <div>
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
    []);

  // Focus on / or . key press.
  useEffect(() => {
    let l = (event: any) => {
      if ('ArrowUp' === event.key)
        setSelectedResultIndex(selectedResultIndex - 1);
      if ('ArrowDown' === event.key)
        setSelectedResultIndex(selectedResultIndex + 1);
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
    label={query.length ? "" : "Search (Tap / or . to focus)"}
    variant="outlined"
    size={query.length ? "medium" : "small"}
    // variant="filled"
    InputProps={{
      id:"doc_search_bar",
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
