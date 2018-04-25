\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{clt2011}[2010/09/14 CLT class]
\LoadClass[a5paper,smallheadings,twoside]{scrartcl}
%
%\setlength{\paperwidth}{210mm}
%\setlength{\paperheight}{297mm}
\RequirePackage[T1]{fontenc}
\RequirePackage[9pt]{extsizes}
\RequirePackage{url}
\RequirePackage{mathptmx}
\RequirePackage[
 a5paper,
 left=1.5cm,
 right=1.5cm,
 top=1.5cm,
 bottom=2.0cm
]{geometry}
\pagestyle{empty}
\renewcommand*{\titlepagestyle}{empty}
\pagenumbering{arabic}
\urlstyle{sf}
\renewenvironment{abstract}{%
\centerline{\sectfont\abstractname}
\begin{quotation}
\small\noindent
}{
\end{quotation}
}
\newcommand{\titleemail}[1]{\small\sffamily\mdseries #1}
\newcommand{\titlewww}[1]{\small\sffamily\mdseries \url{#1}}
% Redefine @maketitle

\renewcommand*{\@maketitle}{%
  \clearpage
  \let\footnote\thanks
  \begin{center}%
    {\titlefont\Large \@title \par}%
    \vskip 1.0em%
    {\titlefont\normalsize
      \lineskip .5em%
      \begin{tabular}[t]{c}
        \@author
      \end{tabular}\par
    }%
    \vskip 1em
    {\titlefont\normalsize \@publishers \par}
  \end{center}%
  \par
  \vskip 1em
}

\renewcommand{\tableofcontents}{}
% paragraph handling
\setlength{\parskip}{2ex plus1ex minus0.5ex}
\setlength{\parindent}{0ex}
% Redefine abstract (no indent)
\renewenvironment{abstract}{%
  \if@twocolumn\if@abstrt
      \addsec*{\abstractname}
    \fi
  \else
    \if@abstrt
      \small
      \begin{center}
        {\normalfont\sectfont\nobreak\abstractname
          \vspace{-.5em}\vspace{\z@}}%
      \end{center}
    \fi
    \small
    \quotation\noindent\ignorespaces
  \fi
}{%
\if@twocolumn\else\endquotation\fi
}
\endinput

