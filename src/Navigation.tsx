import React, { createContext, PropsWithChildren, useCallback, useContext, useEffect, useState } from "react";
import Link, { LinkProps } from "@material-ui/core/Link"
import { Button, ButtonProps } from "@material-ui/core";
import _ from "lodash";

let navigationContext = createContext({
  path: "/",
  navigateTo: (path: string) => { }
})
let navigationToContext = createContext({
  navigateTo: (path: string) => { }
})

let onUrlChangeCallbacks: ((path: string) => void)[] = [];

declare global {
  var navigateTo: (url: string) => void;
}
window.navigateTo = () => { };

export const Navigation = (props: PropsWithChildren<{}>) => {

  const [path, setPath] = useState(window.location.pathname);

  const navigateTo = useCallback(
    (pathname: string) => {
      window.history.pushState({}, "", pathname);
      setPath(pathname);
      onUrlChangeCallbacks.forEach(f => f(pathname));
    },
    [],
  );
  window.navigateTo = navigateTo;

  useEffect(() => {
    window.onpopstate = () => {
      setPath(window.location.pathname);
    }
  }, [path]);

  return <navigationContext.Provider value={{
    navigateTo,
    path
  }}>
    <navigationToContext.Provider value={{
      navigateTo,
    }}>
      {props.children}
    </navigationToContext.Provider>
  </navigationContext.Provider>
}

export function useNavigation() {
  return useContext(navigationContext);
}
export function useNavigateTo() {
  return useContext(navigationToContext);
}
export function useOnUrlChange(f: (path: string) => void) {
  useEffect(() => {
    onUrlChangeCallbacks.push(f);
    f(window.location.pathname);
    return () => {
      _.remove(onUrlChangeCallbacks, cb => f === cb);
    }
  }, [f]);
}

export const preventDefault = { onClick: (e: any) => e.preventDefault() };

export const hrefPrefix = (() => {
  console.log(process.env.PUBLIC_URL);
  let p = process.env.PUBLIC_URL;
  p = p.replace(/\/$/, "");
  return p;
})();

export function makeHrefUrl(path : string|null) {
  let p = path || "";
  if (p.startsWith("https://"))
    return p;
  if (p.startsWith("/"))
    return hrefPrefix + p;
  else
    return hrefPrefix + "/" + p;
}
export function isSameUrl(url1 : string, url2 : string) {
  return url1.replace(/\/$/, "") === url2.replace(/\/$/, "");
}

export const NavigationLink = (props: LinkProps) => {
  // const nav = useNavigateTo();
  const url = makeHrefUrl(props.href || "");
  return <Link {...props} href={url} onClick={(e: any) => { props.onClick?.(e); e.preventDefault(); window.navigateTo(url); }}>
    {props.children}
  </Link>
};

export const NavigationButton = (props: ButtonProps) => {
  const url = makeHrefUrl(props.href || "");
  // const nav = useNavigateTo();

  return <Button {...props} href={url} onClick={(e) => { props.onClick?.(e); e.preventDefault(); window.navigateTo(url); }}>
    {props.children}
  </Button>
};
