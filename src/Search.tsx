import { TextField, Typography } from "@material-ui/core";
import Icon from "@material-ui/core/Icon/Icon";
import InputAdornment from "@material-ui/core/InputAdornment/InputAdornment";
import Autocomplete, { AutocompleteRenderOptionState } from '@material-ui/lab/Autocomplete';
import { FilterOptionsState } from "@material-ui/lab/useAutocomplete";
import _ from "lodash";
import React, { useEffect, useState } from "react";
import { DocIndex, DocIndexEntry, documentationIndex, sectionAnchor, sectionPath } from "./Documentation";

// const filterOptions = createFilterOptions({
//   matchFrom: 'any',
//   ignoreAccents: true,
//   ignoreCase: true,
//   stringify: (option: DocIndexEntry) => option.text,
// });

function filterOptions(options: DocIndexEntry[], state: FilterOptionsState<DocIndexEntry>) : DocIndexEntry[] {

  let queryWords = state.inputValue.split(" ").filter(s => s.length !== 0).map(s => s.toLowerCase());
  let selected : [DocIndexEntry, number][] = [];
  // return options.filter(entry => {
  //   for (let w of queryWords)
  //   // if (entry.text.toLowerCase().includes(w) || sectionPath(entry.section).toLowerCase().includes(w))
  //     if (entry.text.indexOf(w) !== -1)
  //       return true;
  //   return false;
  // });
  for (let entry of options)
  {
    let score = 0;
    for (let w of queryWords)
    {
      if (entry.text.toLowerCase().includes(w))
        score++;
      else if (entry.section.text.toLowerCase().includes(w))
        score += 2;
    }
    if (score > 0) selected.push([entry, score]);
  }
  selected = _.uniqBy(selected, (s) => s[0].section);
  selected = _.sortBy(selected, (s) => -s[1]);

  return selected.map(s => s[0]).slice(0, 10);
}

export const Search = () => {

  const [index, setIndex] = useState<DocIndex | null>(null)

  useEffect(() => {
    let l = (event : any) => {
      if (['/', '.'].includes(event.key))
        document.getElementById('doc_search_bar')?.focus();
    };
    
    document.addEventListener('keyup', l, false);

    return () => { document.removeEventListener('keyup', l); }  
  }, []);

  useEffect(() => {
    documentationIndex.then(index => {
      setIndex(index[1]);
    });
  }, []);

  if (index)
    return <Autocomplete
      id="doc_search_bar"
      options={index}
      style={{ width: 500 }}
      openOnFocus={false}
      // onOpen={e => { console.log(e); e.preventDefault(); return false; } }
      renderInput={(params) =>  <TextField {...params}
      onMouseDownCapture={(e) => e.stopPropagation()}
      InputProps={{
        ...params.InputProps,
          // style: { color: theme.palette.common.black, borderColor: theme.palette.common.black },
          startAdornment:
            <InputAdornment position="start">
              <Icon>search</Icon>
            </InputAdornment>
          ,
        }}
        label="Search (Tap / or . to focus)"
        // InputLabelProps={{ style: { color: theme.palette.common.black } }}
        // style={{ color: theme.palette.common.black, borderColor: theme.palette.common.black }}
        variant="outlined"
        size="small"
        // variant="filled"
      />}
      filterOptions={filterOptions}
      onChange={(e, option) => {
        
        if (option) {
          window.location.hash = sectionAnchor(option.section);
        }
      }}
      getOptionLabel={(option) => ""}
      renderOption={(option: DocIndexEntry, state: AutocompleteRenderOptionState) => {

        let queryWords = state.inputValue.split(" ").filter(s => s.length !== 0);

        let queryWordsRegexp = queryWords.map(w => w.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')).join('|');

        let indices = queryWords.map(w => option.text.toLowerCase().indexOf(w.toLowerCase())).filter(i => i !== -1);
        let indicesGroups = indices.reduce<number[][]>((acc : number[][], cur : number, curIdx : number) => {
          if (curIdx === 0) return acc;
          if (cur - (_.last(acc) as number[])[0] < 100) {
            _.last(acc)?.push(cur);
            return acc;
          }
          else
          return [...acc, [cur]];
        }, [[indices[0]]]);
      
        let snippets : string[] = indicesGroups.map(g => {
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
        return <div key={sectionPath(option.section) + option.text} style={{ display: "flex", flexDirection: "column", borderLeft: "1px solid #999999", paddingLeft: "10px", width: "600px" }}>
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
      }}
    />

  else
    return <></>
}
