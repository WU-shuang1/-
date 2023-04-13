" multi-encoding setting
"if has("multi_byte")
"set bomb
"set fileencodings=ucs-bom,utf-8,cp936,big5,euc-jp,euc-kr,latin1
" CJK environment detection and corresponding setting
"if v:lang =~ "^zh_CN"
" Use cp936 to support GBK, euc-cn == gb2312
"set encoding=cp936
"set termencoding=cp936
"set fileencoding=cp936
"elseif v:lang =~ "^zh_TW"
" cp950, big5 or euc-tw
" Are they equal to each other?
"set encoding=big5
"set termencoding=big5
"set fileencoding=big5
"elseif v:lang =~ "^ko"
" Copied from someone's dotfile, untested
"set encoding=euc-kr
"set termencoding=euc-kr
"set fileencoding=euc-kr
"elseif v:lang =~ "^ja_JP"
" Copied from someone's dotfile, untested
"set encoding=euc-jp
"set termencoding=euc-jp
"set fileencoding=euc-jp
"endif
" Detect UTF-8 locale, and replace CJK setting if needed
"if v:lang =~ "utf8$" || v:lang =~ "UTF-8$"
"set encoding=utf-8
"set termencoding=utf-8
"set fileencoding=utf-8
"endif
"else
"echoerr "Sorry, this version of (g)vim was not compiled with multi_byte"
"endif
set mouse=v
set tabstop=4 sw=4
"set expandtab
set shiftwidth=4
set nu
set autoindent
set smartindent
set incsearch
set tags=tags;
set showmatch
set hlsearch

"自动补全括号
"inoremap ( ()<ESC>i
"inoremap [ []<ESC>i
"inoremap { {}<ESC>i
"inoremap ' ''<ESC>i
"inoremap " ""<ESC>i

"leader键修改
let mapleader = ","

imap <C-b> <ESC>:Autoformat<CR>:w<CR>
nmap <C-b> <ESC>:w<CR>
nmap <C-a> :q<CR>

"翻页
nmap <F3> :bp<CR>
nmap <F4> :bn<CR>

"取消光标闪烁
set gcr=a:block-blinkon0
"  突出显示当前行
set cursorline
" 突出显示当前列
set cursorcolumn

" # 设置vim打开文件，光标保持最下面有5行，最上面也是一样的
set scrolloff=8

"定位到行末尾
imap <C-Home> <End>
nmap <C-Home> <End>
imap <C-\> <End><CR>

" 解决Backspace无法删除问题
set backspace=2
syntax on
" Uncomment the following to have Vim jump to the last position when
" " reopening a file
if has("autocmd")
	au BufReadPost * if line("'\"") > 1 && line("'\"") <= line("$") | exe "normal! g'\"" | endif
endif

" Open and close all the three plugins on the same time
"nmap <F8>   :TrinityToggleAll<CR> :set mouse=a<CR> :TrinityToggleSourceExplorer<CR>

" Open and close the srcexpl.vim separately
"nmap <F9>   :TrinityToggleSourceExplorer<CR>

" Open and close the taglist.vim separately
"nmap <F10>  :TrinityToggleTagList<CR>

" Open and close the NERD_tree.vim separately
"nmap <F11>  :TrinityToggleNERDTree<CR>
nmap <F7>  :set mouse=v<CR>
nmap <F6>  :set mouse=a<CR>
"nmap <c-o> o<ESC>k
"nmap <c-h> i<CR><ESC>k$
"nmap m jjjjj
"nmap , kkkkk
"set fdm=syntax

" // The switch of the Source Explorer
"nmap <F8> :SrcExplToggle<CR>

" // Set the height of Source Explorer window
"et g:SrcExpl_winHeight = 8

" // Set 100 ms for refreshing the Source Explorer
"et g:SrcExpl_refreshTime = 100

" // Set "Enter" key to jump into the exact definition context
"et g:SrcExpl_jumpKey = "<ENTER>"

" // Set "Space" key for back from the definition context
"et g:SrcExpl_gobackKey = "<SPACE>"

" // In order to Avoid conflicts, the Source Explorer should know what plugins
" // are using buffers. And you need add their bufname into the list below
" // according to the command ":buffers!"
"et g:SrcExpl_pluginList = [
			\ "__Tag_List__",
			\ "_NERD_tree_",
			\ "Source_Explorer"
			\ ]
" // Enable/Disable the local definition searching, and note that this is not
" // guaranteed to work, the Source Explorer doesn't check the syntax for now.
" // It only searches for a match with the keyword according to command 'gd'
"et g:SrcExpl_searchLocalDef = 1

" // Let the Source Explorer update the tags file when opening
"et g:SrcExpl_isUpdateTags = 1

" // Use program 'ctags' with argument '--sort=foldcase -R' to create or
" // update a tags file
"et g:SrcExpl_updateTagsCmd = "ctags --sort=foldcase -R ."

" // Set "<F12>" key for updating the tags file artificially
"et g:SrcExpl_updateTagsKey = "<F12>"

"插件
call plug#begin('~/.vim/plugged')
"树形目录
Plug 'preservim/nerdtree' |
			\ Plug 'Xuyuanp/nerdtree-git-plugin' |
			\ Plug 'ryanoasis/vim-devicons'

"状态栏
Plug 'vim-airline/vim-airline'
Plug 'vim-airline/vim-airline-themes'

"静态代码纠错
Plug 'scrooloose/syntastic'

"主题设置
Plug 'ghifarit53/tokyonight-vim'

"代码格式化
Plug 'chiel92/vim-autoformat'

"智能补全
Plug 'ycm-core/YouCompleteMe'

"彩色括号
Plug 'kien/rainbow_parentheses.vim'

"注释插件
Plug 'scrooloose/nerdcommenter' "

"自动补全括号
Plug 'jiangmiao/auto-pairs'

"显示函数
Plug 'vim-scripts/taglist.vim'


" 中文文档
Plug 'yianwillis/vimcdoc'

"树形目录美化
Plug 'tiagofumo/vim-nerdtree-syntax-highlight'

call plug#end()

"静态代码纠错
set statusline+=%#warningmsg#
set statusline+=%{SyntasticStatuslineFlag()}
set statusline+=%*

let g:syntastic_always_populate_loc_list = 1
let g:syntastic_auto_loc_list = 1
let g:syntastic_check_on_open = 1
let g:syntastic_check_on_wq = 0
let g:syntastic_c_check_header = 1
"静态代码纠错end

"主题设置
set termguicolors
let g:tokyonight_style = 'night' " available: night, storm
let g:tokyonight_enable_italic = 1
colorscheme tokyonight
"主题设置end

"底下状态栏配置
set laststatus=2  "永远显示状态栏
let g:airline_powerline_fonts = 1  " 支持 powerline 字体
let g:airline#extensions#tabline#enabled = 1  "显示窗口tab和buffer
let g:airline_theme='zenburn'  " murmur配色不错

if !exists('g:airline_symbols')
	let g:airline_symbols = {}
endif
let g:airline_left_sep = '▶'
let g:airline_left_alt_sep = '❯'
let g:airline_right_sep = '◀'
let g:airline_right_alt_sep = '❮'
let g:airline_symbols.linenr = '¶'
let g:airline_symbols.branch = '⎇'
"状态栏end

"代码格式化

"代码格式化end

"自动补全
let g:ycm_add_preview_to_completeopt = 0
let g:ycm_show_diagnostics_ui = 0
let g:ycm_server_log_level = 'info'
let g:ycm_min_num_identifier_candidate_chars = 2
let g:ycm_collect_identifiers_from_comments_and_strings = 1
let g:ycm_complete_in_strings=1
let g:ycm_key_invoke_completion = '<c-z>'
set completeopt=menu,menuone

noremap <c-z> <NOP>

let g:ycm_semantic_triggers =  {
			\ 'c,cpp,python,java,go,erlang,perl': ['re!\w{2}'],
			\ 'cs,lua,javascript': ['re!\w{2}'],
			\ }
let g:ycm_filetype_whitelist = {
			\ "c":1,
			\ "cpp":1,
			\ "objc":1,
			\ "sh":1,
			\ "zsh":1,
			\ "zimbu":1,
			\ }
"自动补全end

"彩色括号
let g:rbpt_colorpairs = [
			\ ['brown',       'RoyalBlue3'],
			\ ['Darkblue',    'SeaGreen3'],
			\ ['darkgray',    'DarkOrchid3'],
			\ ['darkgreen',   'firebrick3'],
			\ ['darkcyan',    'RoyalBlue3'],
			\ ['darkred',     'SeaGreen3'],
			\ ['darkmagenta', 'DarkOrchid3'],
			\ ['brown',       'firebrick3'],
			\ ['gray',        'RoyalBlue3'],
			\ ['darkmagenta', 'DarkOrchid3'],
			\ ['Darkblue',    'firebrick3'],
			\ ['darkgreen',   'RoyalBlue3'],
			\ ['darkcyan',    'SeaGreen3'],
			\ ['darkred',     'DarkOrchid3'],
			\ ['red',         'firebrick3'],
			\ ]

let g:rbpt_max = 16
let g:rbpt_loadcmd_toggle = 0
au VimEnter * RainbowParenthesesToggle
au Syntax * RainbowParenthesesLoadRound
au Syntax * RainbowParenthesesLoadSquare
au Syntax * RainbowParenthesesLoadBraces
"彩色括号end

"树形文件夹
map <F2> :NERDTreeToggle<CR>
autocmd VimEnter * NERDTree | wincmd p
"let g:NERDTreeDirArrows = 1
"let g:NERDTreeDirArrowExpandable = '+'
"let g:NERDTreeDirArrowCollapsible = '-'
let g:NERDTreeDirArrowExpandable = '▸'
let g:NERDTreeDirArrowCollapsible = '▾'
let NERDTreeAutoCenter=1

"set bsdir=buffer
let g:NERDTreeGlyphReadOnly = "RO"
let g:NERDTreeNodeDelimiter = "\u00b0"
let g:NERDTreeWinSize = 30 "设定 NERDTree 视窗大小
let g:NERDTreeHidden=0     "不显示隐藏文件
let NERDChristmasTree=1
autocmd bufenter * if (winnr("$") == 1 && exists("b:NERDTree") && b:NERDTree.isTabTree()) | q | endif
"树形文件夹end

"注释插件

"注释end

"显示函数
map <F5> :TlistToggle<CR>
let Tlist_Show_One_File = 1		"不同时显示多个文件的tag，只显示当前文件的
let Tlist_Exit_OnlyWindow = 1	"如果taglist窗口是最后一个窗口，则退出vim
let Tlist_Use_Right_Window = 1	"在右侧窗口中显示taglist窗口
let Tlist_Auto_Open=1
let g:Tlist_WinWidth=30

fun! NoExcitingBuffersLeft()
	if tabpagenr("$") == 1 && winnr("$") == 2
		let window1 = bufname(winbufnr(1))
		let window2 = bufname(winbufnr(2))
		if (window1 == t:NERDTreeBufName && window2 =="__Tag_List__")
			quit
		endif
	endif
endfun
au WinEnter * call NoExcitingBuffersLeft()
"显示函数end

"树形目录美化
"let g:webdevicons_enable_nerdtree = 1
let g:airline_powerline_fonts = 1
set guifont=BitetreamVeraSansMono_Nerd_Font_Mono_Roman:h12
"树形目录美化end
