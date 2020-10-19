import { TextField, Typography, useTheme } from "@material-ui/core"
import React, { useEffect, useState } from "react"
import { DocIndex, DocIndexEntry, documentationIndex, sectionAnchor, sectionPath, sectionUrl } from "./Documentation"
import Autocomplete, { AutocompleteRenderOptionState, createFilterOptions } from '@material-ui/lab/Autocomplete';
import { options } from "marked";
import { useHistory } from "react-router-dom";
import InputAdornment from "@material-ui/core/InputAdornment/InputAdornment";
import Icon from "@material-ui/core/Icon/Icon";
import { DesktopWindows } from "@material-ui/icons";

const filterOptions = createFilterOptions({
  matchFrom: 'any',
  ignoreAccents: true,
  ignoreCase: true,
  stringify: (option: DocIndexEntry) => option.text,
});

export const Search = () => {

  const history = useHistory();

  const [index, setIndex] = useState<DocIndex | null>(null)

  useEffect(() => {
    documentationIndex.then(index => {
      console.log(index[1]);
      setIndex(index[1]);
    });
  }, []);

  const theme = useTheme();

  if (index)
    return <Autocomplete
      id="doc_search_bar"
      options={index}
      style={{ width: 600 }}
      openOnFocus={false}
      // onOpen={e => { console.log(e); e.preventDefault(); return false; } }
      renderInput={(params) => <TextField {...params}

      onMouseDownCapture={(e) => e.stopPropagation()}
      // InputProps={{
        //   // style: { color: theme.palette.common.black, borderColor: theme.palette.common.black },
        //   startAdornment: (
        //     <InputAdornment position="start">
        //       <Icon>search</Icon>
        //     </InputAdornment>
        //   ),
        // }}
        label="Search"
        // InputLabelProps={{ style: { color: theme.palette.common.black } }}
        // style={{ color: theme.palette.common.black, borderColor: theme.palette.common.black }}
        variant="outlined"

      />}
      filterOptions={filterOptions}
      onChange={(e, option) => {
        console.log("selected!");
        console.log(option);

        if (option) {
          window.location.hash = sectionAnchor(option.section);
        }
      }}
      getOptionLabel={(option) => ""}
      renderOption={(option: DocIndexEntry, state: AutocompleteRenderOptionState) => {
        
        let idx = option.text.toLowerCase().indexOf(state.inputValue.toLowerCase());
        // Take 100 chars.
        let snippet = option.text.substring(Math.max(idx - 50, 0), Math.min(idx + 50, option.text.length));
        let highlight = (str : string) => {
          return str.replace(new RegExp(`(${state.inputValue})`, 'gi'), `<span class="searchHighlight">$1</span>`);
        };
        return <div key={sectionPath(option.section) + option.text} style={{ display: "flex", flexDirection: "column", borderLeft: "1px solid #999999", paddingLeft: "10px" }}>
          {
            option.section.parent ? <div>
              <Typography variant="caption">{sectionPath(option.section)}</Typography>
            </div> : <></>
          }
          <div>
            <Typography variant="h6"><div  dangerouslySetInnerHTML={{ __html: highlight(option.section.text)}} /></Typography>
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
