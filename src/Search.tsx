import { TextField, Typography, useTheme } from "@material-ui/core"
import React, { useEffect, useState } from "react"
import { DocIndex, DocIndexEntry, documentationIndex, sectionPath, sectionUrl } from "./Documentation"
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
      renderInput={(params) => <TextField {...params}
        // InputProps={{
        //   // style: { color: theme.palette.common.black, borderColor: theme.palette.common.black },
        //   startAdornment: (
        //     <InputAdornment position="start">
        //       <Icon>search</Icon>
        //     </InputAdornment>
        //   ),
        // }}
        label="Search documentation"
        // InputLabelProps={{ style: { color: theme.palette.common.black } }}
        // style={{ color: theme.palette.common.black, borderColor: theme.palette.common.black }}
        variant="outlined"

      />}
      filterOptions={filterOptions}
      onChange={(e, option) => {
        console.log("selected!");
        console.log(option);

        if (option) {
          let url = sectionUrl(option?.section)
          let { pathname, hash } = new URL("http://test.com" + url);

          if (window.location.pathname != pathname)
            history.push(url.toString());
          else
            window.location.hash = hash;
        }
      }}
      getOptionLabel={(option) => ""}
      renderOption={(option: DocIndexEntry, state: AutocompleteRenderOptionState) => {

        console.log(state.inputValue);
        let idx = option.text.indexOf(state.inputValue);
        // Take 100 chars.
        let snippet = option.text.substring(Math.max(idx - 50, 0), Math.min(idx + 50, option.text.length));
        snippet = snippet.replace(state.inputValue, `<u><b>${state.inputValue}</b></u>`)
        return <div key={sectionPath(option.section) + option.text} style={{ display: "flex", flexDirection: "column", borderLeft: "1px solid #999999", paddingLeft: "10px" }}>
          {
            option.section.parent ? <div>
              <Typography variant="caption">{sectionPath(option.section)}</Typography>
            </div> : <></>
          }
          <div>
            <Typography variant="h6">{option.section.text}</Typography>
          </div>
          <div>
            <div dangerouslySetInnerHTML={{ __html: snippet }}></div>
          </div>
        </div>
      }}
    />

  else
    return <></>
}
