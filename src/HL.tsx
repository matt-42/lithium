import hljs from 'highlight.js'
import "highlight.js/styles/vs2015.css"
import React, { PropsWithChildren } from 'react';
import { useRef, useEffect } from 'react';

export const codeFont = {
  fontFamily: "\"Droid Sans Mono\", monospace, monospace, \"Droid Sans Fallback\"",
  fontSize: "14px"
}


export const HL = (props: PropsWithChildren<{language : string}>) => {

  const el = useRef<HTMLPreElement>(null);

  useEffect(() => {
    if (!el.current) return;
    hljs.highlightBlock(el.current);
  }, [el.current !== null]);

  return <pre><code ref={el} className={props.language}>{props.children}</code></pre>
}


export const HLsh = (props: PropsWithChildren<{}>) => {

  const el = useRef<HTMLPreElement>(null);

  useEffect(() => {
    if (!el.current) return;
    hljs.highlightBlock(el.current);
  }, [el.current !== null]);

  return <pre><code ref={el} className="shell">{props.children}</code></pre>
}


export const HLcpp = (props: PropsWithChildren<{}>) => {

  const el = useRef<HTMLPreElement>(null);

  useEffect(() => {
    if (!el.current) return;
    hljs.highlightBlock(el.current);
  }, [el.current !== null]);

  return <pre><code ref={el} className="c++">{props.children}</code></pre>
}

export const InlineCode = (props: PropsWithChildren<{}>) => {
  return <span style={{ fontFamily: codeFont.fontFamily, fontWeight: "bold" }}>{props.children}</span>
}