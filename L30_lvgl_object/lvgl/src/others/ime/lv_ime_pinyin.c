/**
 * @file lv_ime_pinyin.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_ime_pinyin_private.h"
#include "../../core/lv_obj_class_private.h"
#if LV_USE_IME_PINYIN != 0

#include "../../lvgl.h"
#include "../../core/lv_global.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS (&lv_ime_pinyin_class)
#define cand_len LV_GLOBAL_DEFAULT()->ime_cand_len

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_ime_pinyin_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_ime_pinyin_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_ime_pinyin_style_change_event(lv_event_t * e);
static void lv_ime_pinyin_kb_event(lv_event_t * e);
static void lv_ime_pinyin_cand_panel_event(lv_event_t * e);

static void init_pinyin_dict(lv_obj_t * obj, const lv_pinyin_dict_t * dict);
static void pinyin_input_proc(lv_obj_t * obj);
static void pinyin_page_proc(lv_obj_t * obj, uint16_t btn);
static char * pinyin_search_matching(lv_obj_t * obj, char * py_str, uint16_t * cand_num);
static void pinyin_ime_clear_data(lv_obj_t * obj);

#if LV_IME_PINYIN_USE_K9_MODE
    static void pinyin_k9_init_data(lv_obj_t * obj);
    static void pinyin_k9_get_legal_py(lv_obj_t * obj, char * k9_input, const char * py9_map[]);
    static bool pinyin_k9_is_valid_py(lv_obj_t * obj, char * py_str);
    static void pinyin_k9_fill_cand(lv_obj_t * obj);
    static void pinyin_k9_cand_page_proc(lv_obj_t * obj, uint16_t dir);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

const lv_obj_class_t lv_ime_pinyin_class = {
    .constructor_cb = lv_ime_pinyin_constructor,
    .destructor_cb  = lv_ime_pinyin_destructor,
    .width_def      = LV_SIZE_CONTENT,
    .height_def     = LV_SIZE_CONTENT,
    .group_def      = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size  = sizeof(lv_ime_pinyin_t),
    .base_class     = &lv_obj_class,
    .name = "lv_ime_pinyin",
};

#if LV_IME_PINYIN_USE_K9_MODE
static const char * lv_btnm_def_pinyin_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 21] = {\
                                                                                      ",\0", "123\0",  "abc \0", "def\0",  LV_SYMBOL_BACKSPACE"\0", "\n\0",
                                                                                      ".\0", "ghi\0", "jkl\0", "mno\0",  LV_SYMBOL_KEYBOARD"\0", "\n\0",
                                                                                      "?\0", "pqrs\0", "tuv\0", "wxyz\0",  LV_SYMBOL_NEW_LINE"\0", "\n\0",
                                                                                      LV_SYMBOL_LEFT"\0", "\0"
                                                                                     };

static lv_buttonmatrix_ctrl_t default_kb_ctrl_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 17] = { 1 };
static char   lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 2][LV_IME_PINYIN_K9_MAX_INPUT] = {0};
#endif

static char   lv_pinyin_cand_str[LV_IME_PINYIN_CAND_TEXT_NUM][4];
static char * lv_btnm_def_pinyin_sel_map[LV_IME_PINYIN_CAND_TEXT_NUM + 3];

#if LV_IME_PINYIN_USE_DEFAULT_DICT
static const lv_pinyin_dict_t lv_ime_pinyin_def_dict[] = {
    { "a", "ah" },
    { "ai", "love" },
    { "an", "Annan case" },
    { "ba", "Let's take daddy eight" },
    { "bai", "Bai Bai" },
    { "ban", "Half-ordinary" },
    { "bang", "beside" },
    { "bao", "Thin guarantee" },
    { "bei", "Being sad about the North Cup" },
    { "ben", "Book" },
    { "bi", "It is better to avoid the nose and close the pen" },
    { "bian", "The stool changes" },
    { "biao", "Tag" },
    { "bie", "No" },
    { "bing", "Sick and aggravated" },
    { "bo", "Bobo" },
    { "bu", "Step-by-step control" },
    { "ca", "Observe" },
    { "cai", "Talent food and money" },
    { "can", "Ginseng residual" },
    { "ce", "Side" },
    { "ceng", "ever" },
    { "cha", "Failure to check tea" },
    { "chai", "Difference" },
    { "chan", "Production single" },
    { "chang", "Factory" },
    { "chao", "Super dynasty" },
    { "che", "vehicle" },
    { "cheng", "Cheng Cheng" },
    { "chi", "Eat the ruler and hold the red pot of Akachi" },
    { "chong", "Replenish the species" },
    { "chu", "Out of Chu Chu's place" },
    { "chuan", "Sichuan Ship" },
    { "chuang", "Creation" },
    { "chun", "spring" },
    { "ci", "This time I'm leaving" },
    { "cong", "From" },
    { "cu", "Pawn" },
    { "cun", "Save the village" },
    { "cuo", "mistaken" },
    { "da", "Dada answer" },
    { "dai", "Take a sack on behalf of" },
    { "dan", "But be responsible for single" },
    { "dang", "Be a party" },
    { "dao", "Arrived at the thief island" },
    { "de", "It's OK" },
    { "dei", "" },
    { "deng", "wait" },
    { "di", "The best brother is low" },
    { "dian", "Click on the electric store and the electric store" },
    { "diao", "Comment" },
    { "ding", "Dingcho" },
    { "dong", "Dongdong Movement" },
    { "du", "Unique Duduo" },
    { "duan", "Short break" },
    { "dui", "Right" },
    { "duo", "Duo" },
    { "e", "Yes" },
    { "en", "Um" },
    { "er", "And Er Er" },
    { "fa", "Unsuccessful method to develop hair" },
    { "fan", "Reverse revenge crime" },
    { "fang", "Visit Fangfangfangfang" },
    { "fei", "Non-flying charge" },
    { "fen", "Partitions" },
    { "feng", "Feng Feng" },
    { "fou", "No" },
    { "fu", "Father and husband's rich clothes and talismans are repetitive and negative" },
    { "gai", "Change the summary" },
    { "gan", "Delighted" },
    { "gang", "Hong Kong" },
    { "gao", "Let's go high" },
    { "ge", "Each Ge Ge Ge Ge is divided" },
    { "gei", "give" },
    { "gen", "Follow the root" },
    { "geng", "Even" },
    { "gong", "Workers and Workers provide meritorious service" },
    { "gou", "Structure" },
    { "gu", "Ancient Drum" },
    { "guai", "Shrink" },
    { "guan", "Official Administration Court Visitor" },
    { "guang", "Light" },
    { "gui", "Rules" },
    { "guo", "Guoguo Countryside" },
    { "hai", "Child Hai is aware of" },
    { "han", "Han Han" },
    { "hang", "Sailing" },
    { "hao", "Good number" },
    { "he", "Hehe drinks Hehe" },
    { "hei", "Black" },
    { "hen", "very" },
    { "heng", "Walking horizontally" },
    { "hou", "Thick throat symptoms" },
    { "hu", "Huhu Lake Protection" },
    { "hua", "Flower paintings" },
    { "huai", "Plan" },
    { "huan", "The slimming cycle still changes" },
    { "huang", "yellow" },
    { "hui", "Return to the Huizihuihui meeting" },
    { "hun", "Mixed marriage" },
    { "huo", "Live or fire" },
    { "i", "" },
    { "ji", "It is considered that the emergency mechanism is the most urgent and the technology is the collection of the base sacrificial sacrificial system." },
    { "jia", "Home price" },
    { "jian", "Jiejianjian shoulder-viewing and simplicity" },
    { "jiang", "Reduce the harbor" },
    { "jiao", "Calling the teaching of the Jiaojiao Vision Learning" },
    { "jie", "Introduction to the relationship between the students and sisters, all the worlds are the interpretation of the relationship." },
    { "jin", "The banned gold is only added today" },
    { "jing", "Jingjing Jingjing Jingjing Jingjing" },
    { "jiu", "Just a long time wine study" },
    { "ju", "Jujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujujuju" },
    { "jue", "Corner view" },
    { "jun", "all" },
    { "kai", "open" },
    { "kan", "Read the issue" },
    { "kang", "Kang" },
    { "kao", "Test" },
    { "ke", "Keke Keke Guest Class" },
    { "ken", "Ken" },
    { "kong", "Air control" },
    { "kou", "mouth" },
    { "ku", "Bitter" },
    { "kuai", "Express meeting" },
    { "kuang", "Quite" },
    { "kun", "Sleepy" },
    { "kuo", "Suitable for simplicity" },
    { "la", "Pull it down" },
    { "lai", "Come here" },
    { "lao", "The old man falls" },
    { "le", "Leave the cherry" },
    { "lei", "Category" },
    { "leng", "cold" },
    { "li", "Lili Lili's law and leave the house to seek peace" },
    { "lian", "Training face contact" },
    { "liang", "Good quality and squid" },
    { "liao", "material" },
    { "lie", "List" },
    { "lin", "Lin Wan Praise" },
    { "ling", "Another" },
    { "liu", "Six remnants" },
    { "lu", "Ruluzontal Continental Conduct" },
    { "lv", "trip" },
    { "lun", "Wheel discussion" },
    { "luo", "Defeat" },
    { "ma", "Mom," },
    { "mai", "Buy a pot" },
    { "man", "可" },
    { "mang", "busy" },
    { "mao", "Hairy cat pedestal" },
    { "me", "Yes" },
    { "mei", "Beautiful girls never have any media" },
    { "men", "可" },
    { "mi", "Mi Secret" },
    { "mian", "Sleep free from face" },
    { "miao", "Scan" },
    { "min", "Mind dishes" },
    { "ming", "Definition of name" },
    { "mo", "Final model" },
    { "mou", "certain" },
    { "mu", "Male wood eye mold" },
    { "na", "Then where can I take Neinan" },
    { "nan", "Nan Nan" },
    { "nao", "Brain" },
    { "ne", "Then where" },
    { "nei", "Where is it" },
    { "neng", "able" },
    { "ni", "What about you" },
    { "nian", "Year's anniversary" },
    { "niang", "mother" },
    { "nin", "you" },
    { "ning", "Condensation" },
    { "niu", "ox" },
    { "nong", "Agricultural" },
    { "nu", "Female" },
    { "nuan", "warm" },
    { "o", "" },
    { "ou", "未分" },
    { "pa", "Afraid" },
    { "pai", "Forced platoon" },
    { "pan", "Qualification" },
    { "pang", "beside" },
    { "pei", "match" },
    { "peng", "friend" },
    { "pi", "Fatigue" },
    { "pian", "Popular" },
    { "pin", "Pinca" },
    { "ping", "Evaluation" },
    { "po", "Forced Popo" },
    { "pu", "General" },
    { "qi", "The seven-day period of the Qi and the wife's friendship" },
    { "qian", "Inlaid Qianqian Front Cushion" },
    { "qiang", "Strong" },
    { "qiao", "Bridge" },
    { "qie", "And get close" },
    { "qin", "Ding Qin" },
    { "qing", "Qingqing Qingqing Qingqing Qingqing Qingqing please call" },
    { "qiu", "Ask for autumn ball" },
    { "qu", "Go to the fun plot" },
    { "quan", "All dog coupons" },
    { "que", "Missing" },
    { "ran", "Of course" },
    { "rang", "let" },
    { "re", "hot" },
    { "ren", "People can be recognized" },
    { "reng", "still" },
    { "ri", "day" },
    { "rong", "Allow" },
    { "rou", "Weak as meat" },
    { "ru", "If you enter" },
    { "ruan", "Software" },
    { "sai", "Contest" },
    { "san", "three" },
    { "sao", "Talk" },
    { "se", "color" },
    { "sen", "南" },
    { "sha", "sand" },
    { "shan", "Shanshan" },
    { "shang", "Shangshangshang" },
    { "shao", "A few" },
    { "shaung", "pair" },
    { "she", "Social shooting" },
    { "shei", "who" },
    { "shen", "Shi Shenshen is very strong and he is very calm" },
    { "sheng", "Sound and joyful sound" },
    { "shi", "It is a test of the current affairs ten stones that were used to make the world's real history room, Shishishishishishi's Shishishishi's Sisters' test to make the right and practical" },
    { "shou", "Hands' head guard received the award" },
    { "shu", "Shushu Sushi Special Tree Booklet" },
    { "shui", "Who is water talking about" },
    { "shuo", "Numbers" },
    { "si", "Sisi Si Si seems to be dead" },
    { "song", "deliver" },
    { "su", "Susu Susu" },
    { "suan", "Calculate acid" },
    { "sui", "Despite the age" },
    { "sun", "Yes" },
    { "suo", "Place" },
    { "ta", "She, he, it" },
    { "tai", "Taitai tai" },
    { "tan", "Discussion" },
    { "tang", "sugar" },
    { "tao", "Peach escape trap" },
    { "te", "special" },
    { "ti", "Body replacement question" },
    { "tian", "Tiantian" },
    { "tiao", "Articles" },
    { "tie", "Boat" },
    { "ting", "Tingting Honcho" },
    { "tong", "Tongtong pain system" },
    { "tou", "Popular" },
    { "tu", "Tutu tea" },
    { "tuan", "Group" },
    { "tui", "Reject" },
    { "tuo", "Take off" },
    { "u", "" },
    { "v", "" },
    { "wai", "outside" },
    { "wan", "End of Wandering Wrist Gallery" },
    { "wang", "Forgetful hope to die in the Internet" },
    { "wei", "The dangerous position is not worthy of being a surviving enemy" },
    { "wen", "Wen Wen asked" },
    { "wo", "I" },
    { "wu", "Noon things, no house, dead birds" },
    { "xi", "Xixixixi's Xixixi's knees and fine chores" },
    { "xia", "Low summer and narrow leisure" },
    { "xian", "First limit the scandal line of scandal" },
    { "xiang", "Detailed remarks towards Xiangxiang's imagination" },
    { "xiao", "Xiaoxiao eliminates the effect and cuts the slightest" },
    { "xie", "Write some evil-relieving equipment to write a letter" },
    { "xin", "A new heart" },
    { "xing", "The star" },
    { "xiong", "Brother" },
    { "xiu", "Xiu Xiu" },
    { "xu", "Must be allowed to continue" },
    { "xuan", "Choose" },
    { "xue", "Learn to learn how to peel boots in snow" },
    { "xun", "Training" },
    { "ya", "Yes, yes" },
    { "yan", "Yanyan Smoking Serious Train Experience" },
    { "yang", "The style of Yangyang" },
    { "yao", "Want to bend your waist" },
    { "ye", "Yeye Ye evil spirit" },
    { "yi", "Yi Yiyao Yifan's medical expression" },
    { "yin", "Because of the sound of the sound of the drink" },
    { "ying", "Yingying Film should be launched" },
    { "yong", "Use swimming forever" },
    { "you", "There is also a Youyouyou tour email to you" },
    { "yu", "I am thirsty and rainy to heal Yuyuyu language in fish and relationship" },
    { "yuan", "Yuanharayuan Academy Member Encyclopedia Far Ape" },
    { "yue", "Monthly crossing" },
    { "yun", "Yunjiufu" },
    { "za", "只" },
    { "zai", "Reloading" },
    { "zang", "南" },
    { "zao", "Early creation" },
    { "ze", "Being stolen" },
    { "zen", "How" },
    { "zeng", "Have increased" },
    { "zha", "Letter" },
    { "zhai", "Homestay" },
    { "zhan", "Station show battle" },
    { "zhang", "Long-term barrier" },
    { "zhao", "Looking for a chance" },
    { "zhe", "This" },
    { "zhen", "Really shocking" },
    { "zheng", "Confrontation and political struggle" },
    { "zhi", "Only know how to stop and control the value of the Zhili paper quality magazine organization knowledge" },
    { "zhong", "The most important species of medium species" },
    { "zhou", "Zhouzhou Daytime" },
    { "zhu", "Help the main body to live in the pillars and plants to pay attention to the master's authority" },
    { "zhuan", "Specialty" },
    { "zhuang", "Status" },
    { "zhui", "Chase" },
    { "zhun", "未分" },
    { "zhuo", "Put" },
    { "zi", "Zizi's name is Vivian" },
    { "zong", "All" },
    { "zuo", "Left do yesterday's seat" },
    { "zu", "The ancestral tribe team" },
    { "zui", "The most savory" },
    { "zou", "Walk" },
    {NULL, NULL}
};
#endif

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_obj_t * lv_ime_pinyin_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/*=====================
 * Setter functions
 *====================*/

void lv_ime_pinyin_set_keyboard(lv_obj_t * obj, lv_obj_t * kb)
{
    if(kb) {
        LV_ASSERT_OBJ(kb, &lv_keyboard_class);
    }

    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    pinyin_ime->kb = kb;
    lv_obj_set_parent(obj, lv_obj_get_parent(kb));
    lv_obj_set_parent(pinyin_ime->cand_panel, lv_obj_get_parent(kb));
    lv_obj_add_event_cb(pinyin_ime->kb, lv_ime_pinyin_kb_event, LV_EVENT_VALUE_CHANGED, obj);
    lv_obj_align_to(pinyin_ime->cand_panel, pinyin_ime->kb, LV_ALIGN_OUT_TOP_MID, 0, 0);
}

void lv_ime_pinyin_set_dict(lv_obj_t * obj, lv_pinyin_dict_t * dict)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    init_pinyin_dict(obj, dict);
}

void lv_ime_pinyin_set_mode(lv_obj_t * obj, lv_ime_pinyin_mode_t mode)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    LV_ASSERT_OBJ(pinyin_ime->kb, &lv_keyboard_class);

    pinyin_ime->mode = mode;

#if LV_IME_PINYIN_USE_K9_MODE
    if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
        pinyin_k9_init_data(obj);
        lv_keyboard_set_map(pinyin_ime->kb, LV_KEYBOARD_MODE_USER_1, (const char **)lv_btnm_def_pinyin_k9_map,
                            default_kb_ctrl_k9_map);
        lv_keyboard_set_mode(pinyin_ime->kb, LV_KEYBOARD_MODE_USER_1);
    }
#endif
}

/*=====================
 * Getter functions
 *====================*/

lv_obj_t * lv_ime_pinyin_get_kb(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->kb;
}

lv_obj_t * lv_ime_pinyin_get_cand_panel(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->cand_panel;
}

const lv_pinyin_dict_t * lv_ime_pinyin_get_dict(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->dict;
}

/*=====================
 * Other functions
 *====================*/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_ime_pinyin_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    uint16_t py_str_i = 0;
    uint16_t btnm_i = 0;
    for(btnm_i = 0; btnm_i < (LV_IME_PINYIN_CAND_TEXT_NUM + 3); btnm_i++) {
        if(btnm_i == 0) {
            lv_btnm_def_pinyin_sel_map[btnm_i] = "<";
        }
        else if(btnm_i == (LV_IME_PINYIN_CAND_TEXT_NUM + 1)) {
            lv_btnm_def_pinyin_sel_map[btnm_i] = ">";
        }
        else if(btnm_i == (LV_IME_PINYIN_CAND_TEXT_NUM + 2)) {
            lv_btnm_def_pinyin_sel_map[btnm_i] = "";
        }
        else {
            lv_pinyin_cand_str[py_str_i][0] = ' ';
            lv_btnm_def_pinyin_sel_map[btnm_i] = lv_pinyin_cand_str[py_str_i];
            py_str_i++;
        }
    }

    pinyin_ime->mode = LV_IME_PINYIN_MODE_K26;
    pinyin_ime->py_page = 0;
    pinyin_ime->ta_count = 0;
    pinyin_ime->cand_num = 0;
    lv_memzero(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));
    lv_memzero(pinyin_ime->py_num, sizeof(pinyin_ime->py_num));
    lv_memzero(pinyin_ime->py_pos, sizeof(pinyin_ime->py_pos));

    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);

#if LV_IME_PINYIN_USE_DEFAULT_DICT
    init_pinyin_dict(obj, lv_ime_pinyin_def_dict);
#endif

    /* Init pinyin_ime->cand_panel */
    pinyin_ime->cand_panel = lv_buttonmatrix_create(lv_obj_get_parent(obj));
    lv_buttonmatrix_set_map(pinyin_ime->cand_panel, (const char **)lv_btnm_def_pinyin_sel_map);
    lv_obj_set_size(pinyin_ime->cand_panel, LV_PCT(100), LV_PCT(5));
    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);

    lv_buttonmatrix_set_one_checked(pinyin_ime->cand_panel, true);
    lv_obj_remove_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_CLICK_FOCUSABLE);

    /* Set cand_panel style*/
    // Default style
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_0, 0);
    lv_obj_set_style_border_width(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_pad_all(pinyin_ime->cand_panel, 8, 0);
    lv_obj_set_style_pad_gap(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_radius(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_pad_gap(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_base_dir(pinyin_ime->cand_panel, LV_BASE_DIR_LTR, 0);

    // LV_PART_ITEMS style
    lv_obj_set_style_radius(pinyin_ime->cand_panel, 12, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(pinyin_ime->cand_panel, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_0, LV_PART_ITEMS);
    lv_obj_set_style_shadow_opa(pinyin_ime->cand_panel, LV_OPA_0, LV_PART_ITEMS);

    // LV_PART_ITEMS | LV_STATE_PRESSED style
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(pinyin_ime->cand_panel, lv_color_white(), LV_PART_ITEMS | LV_STATE_PRESSED);

    /* event handler */
    lv_obj_add_event_cb(pinyin_ime->cand_panel, lv_ime_pinyin_cand_panel_event, LV_EVENT_VALUE_CHANGED, obj);
    lv_obj_add_event_cb(obj, lv_ime_pinyin_style_change_event, LV_EVENT_STYLE_CHANGED, NULL);

#if LV_IME_PINYIN_USE_K9_MODE
    pinyin_ime->k9_input_str_len = 0;
    pinyin_ime->k9_py_ll_pos = 0;
    pinyin_ime->k9_legal_py_count = 0;
    lv_memzero(pinyin_ime->k9_input_str, LV_IME_PINYIN_K9_MAX_INPUT);

    pinyin_k9_init_data(obj);

    lv_ll_init(&(pinyin_ime->k9_legal_py_ll), sizeof(ime_pinyin_k9_py_str_t));
#endif
}

static void lv_ime_pinyin_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    if(lv_obj_is_valid(pinyin_ime->kb))
        lv_obj_delete(pinyin_ime->kb);

    if(lv_obj_is_valid(pinyin_ime->cand_panel))
        lv_obj_delete(pinyin_ime->cand_panel);
}

static void lv_ime_pinyin_kb_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * kb = lv_event_get_current_target(e);
    lv_obj_t * obj = lv_event_get_user_data(e);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

#if LV_IME_PINYIN_USE_K9_MODE
    static const char * k9_py_map[8] = {"abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"};
#endif

    if(code == LV_EVENT_VALUE_CHANGED) {
        uint16_t btn_id  = lv_buttonmatrix_get_selected_button(kb);
        if(btn_id == LV_BUTTONMATRIX_BUTTON_NONE) return;

        const char * txt = lv_buttonmatrix_get_button_text(kb, lv_buttonmatrix_get_selected_button(kb));
        if(txt == NULL) return;

        lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);

#if LV_IME_PINYIN_USE_K9_MODE
        if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {

            uint16_t tmp_button_str_len = lv_strlen(pinyin_ime->input_char);
            if((btn_id >= 16) && (tmp_button_str_len > 0) && (btn_id < (16 + LV_IME_PINYIN_K9_CAND_TEXT_NUM))) {
                lv_memzero(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));
                lv_strcat(pinyin_ime->input_char, txt);
                pinyin_input_proc(obj);

                for(int index = 0; index < (pinyin_ime->ta_count + tmp_button_str_len); index++) {
                    lv_textarea_delete_char(ta);
                }

                pinyin_ime->ta_count = tmp_button_str_len;
                pinyin_ime->k9_input_str_len = tmp_button_str_len;
                lv_textarea_add_text(ta, pinyin_ime->input_char);

                return;
            }
        }
#endif

        if(lv_strcmp(txt, "Enter") == 0 || lv_strcmp(txt, LV_SYMBOL_NEW_LINE) == 0) {
            pinyin_ime_clear_data(obj);
            lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
        }
        else if(lv_strcmp(txt, LV_SYMBOL_BACKSPACE) == 0) {
            // del input char
            if(pinyin_ime->ta_count > 0) {
                if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K26)
                    pinyin_ime->input_char[pinyin_ime->ta_count - 1] = '\0';
#if LV_IME_PINYIN_USE_K9_MODE
                else
                    pinyin_ime->k9_input_str[pinyin_ime->ta_count - 1] = '\0';
#endif

                pinyin_ime->ta_count--;
                if(pinyin_ime->ta_count <= 0) {
                    pinyin_ime_clear_data(obj);
                    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
                }
                else if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K26) {
                    pinyin_input_proc(obj);
                }
#if LV_IME_PINYIN_USE_K9_MODE
                else if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
                    pinyin_ime->k9_input_str_len = lv_strlen(pinyin_ime->input_char) - 1;
                    pinyin_k9_get_legal_py(obj, pinyin_ime->k9_input_str, k9_py_map);
                    pinyin_k9_fill_cand(obj);
                    pinyin_input_proc(obj);
                    pinyin_ime->ta_count--;
                }
#endif
            }
        }
        else if((lv_strcmp(txt, "ABC") == 0) || (lv_strcmp(txt, "abc") == 0) || (lv_strcmp(txt, "1#") == 0) ||
                (lv_strcmp(txt, LV_SYMBOL_OK) == 0)) {
            pinyin_ime_clear_data(obj);
            return;
        }
        else if(lv_strcmp(txt, "123") == 0) {
            for(uint16_t i = 0; i < lv_strlen(txt); i++)
                lv_textarea_delete_char(ta);

            pinyin_ime_clear_data(obj);
            lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
            lv_ime_pinyin_set_mode(obj, LV_IME_PINYIN_MODE_K9_NUMBER);
            lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
            lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
        }
        else if(lv_strcmp(txt, LV_SYMBOL_KEYBOARD) == 0) {
            if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K26) {
                lv_ime_pinyin_set_mode(obj, LV_IME_PINYIN_MODE_K9);
            }
            else if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
                lv_ime_pinyin_set_mode(obj, LV_IME_PINYIN_MODE_K26);
                lv_keyboard_set_mode(pinyin_ime->kb, LV_KEYBOARD_MODE_TEXT_LOWER);
            }
            else if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9_NUMBER) {
                lv_ime_pinyin_set_mode(obj, LV_IME_PINYIN_MODE_K9);
            }
            pinyin_ime_clear_data(obj);
        }
        else if((pinyin_ime->mode == LV_IME_PINYIN_MODE_K26) && ((txt[0] >= 'a' && txt[0] <= 'z') || (txt[0] >= 'A' &&
                                                                                                      txt[0] <= 'Z'))) {
            uint16_t len = lv_strlen(pinyin_ime->input_char);
            lv_snprintf(pinyin_ime->input_char + len, sizeof(pinyin_ime->input_char) - len, "%s", txt);
            pinyin_input_proc(obj);
            pinyin_ime->ta_count++;
        }
#if LV_IME_PINYIN_USE_K9_MODE
        else if((pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) && (txt[0] >= 'a' && txt[0] <= 'z')) {
            for(uint16_t i = 0; i < 8; i++) {
                if((lv_strcmp(txt, k9_py_map[i]) == 0) || (lv_strcmp(txt, "abc ") == 0)) {
                    if(lv_strcmp(txt, "abc ") == 0)    pinyin_ime->k9_input_str_len += lv_strlen(k9_py_map[i]) + 1;
                    else                            pinyin_ime->k9_input_str_len += lv_strlen(k9_py_map[i]);
                    pinyin_ime->k9_input_str[pinyin_ime->ta_count] = 50 + i;
                    pinyin_ime->k9_input_str[pinyin_ime->ta_count + 1] = '\0';

                    break;
                }
            }
            pinyin_k9_get_legal_py(obj, pinyin_ime->k9_input_str, k9_py_map);
            pinyin_k9_fill_cand(obj);
            pinyin_input_proc(obj);
        }
        else if(lv_strcmp(txt, LV_SYMBOL_LEFT) == 0) {
            pinyin_k9_cand_page_proc(obj, 0);
        }
        else if(lv_strcmp(txt, LV_SYMBOL_RIGHT) == 0) {
            pinyin_k9_cand_page_proc(obj, 1);
        }
#endif
    }
}

static void lv_ime_pinyin_cand_panel_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * cand_panel = lv_event_get_current_target(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_user_data(e);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);
        if(ta == NULL) return;

        uint32_t id = lv_buttonmatrix_get_selected_button(cand_panel);
        if(id == LV_BUTTONMATRIX_BUTTON_NONE) {
            return;
        }
        else if(id == 0) {
            pinyin_page_proc(obj, 0);
            return;
        }
        else if(id == (LV_IME_PINYIN_CAND_TEXT_NUM + 1)) {
            pinyin_page_proc(obj, 1);
            return;
        }

        const char * txt = lv_buttonmatrix_get_button_text(cand_panel, id);
        uint16_t index = 0;
        for(index = 0; index < pinyin_ime->ta_count; index++)
            lv_textarea_delete_char(ta);

        lv_textarea_add_text(ta, txt);

        pinyin_ime_clear_data(obj);
    }
}

static void pinyin_input_proc(lv_obj_t * obj)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    pinyin_ime->cand_str = pinyin_search_matching(obj, pinyin_ime->input_char, &pinyin_ime->cand_num);
    if(pinyin_ime->cand_str == NULL) {
        return;
    }

    pinyin_ime->py_page = 0;

    for(uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++) {
        lv_memset(lv_pinyin_cand_str[i], 0x00, sizeof(lv_pinyin_cand_str[i]));
        lv_pinyin_cand_str[i][0] = ' ';
    }

    // fill buf
    for(uint8_t i = 0; (i < pinyin_ime->cand_num && i < LV_IME_PINYIN_CAND_TEXT_NUM); i++) {
        for(uint8_t j = 0; j < 3; j++) {
            lv_pinyin_cand_str[i][j] = pinyin_ime->cand_str[i * 3 + j];
        }
    }

    lv_obj_remove_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_invalidate(pinyin_ime->cand_panel);
}

static void pinyin_page_proc(lv_obj_t * obj, uint16_t dir)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;
    uint16_t page_num = pinyin_ime->cand_num / LV_IME_PINYIN_CAND_TEXT_NUM;
    uint16_t remainder = pinyin_ime->cand_num % LV_IME_PINYIN_CAND_TEXT_NUM;

    if(!pinyin_ime->cand_str) return;

    if(dir == 0) {
        if(pinyin_ime->py_page) {
            pinyin_ime->py_page--;
        }
    }
    else {
        if(remainder == 0) {
            page_num -= 1;
        }
        if(pinyin_ime->py_page < page_num) {
            pinyin_ime->py_page++;
        }
        else return;
    }

    for(uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++) {
        lv_memset(lv_pinyin_cand_str[i], 0x00, sizeof(lv_pinyin_cand_str[i]));
        lv_pinyin_cand_str[i][0] = ' ';
    }

    // fill buf
    uint16_t offset = pinyin_ime->py_page * (3 * LV_IME_PINYIN_CAND_TEXT_NUM);
    for(uint8_t i = 0; (i < pinyin_ime->cand_num && i < LV_IME_PINYIN_CAND_TEXT_NUM); i++) {
        if((remainder > 0) && (pinyin_ime->py_page == page_num)) {
            if(i >= remainder)
                break;
        }
        for(uint8_t j = 0; j < 3; j++) {
            lv_pinyin_cand_str[i][j] = pinyin_ime->cand_str[offset + (i * 3) + j];
        }
    }
}

static void lv_ime_pinyin_style_change_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    if(code == LV_EVENT_STYLE_CHANGED) {
        const lv_font_t * font = lv_obj_get_style_text_font(obj, LV_PART_MAIN);
        lv_obj_set_style_text_font(pinyin_ime->cand_panel, font, 0);
    }
}

static void init_pinyin_dict(lv_obj_t * obj, const lv_pinyin_dict_t * dict)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    char headletter = 'a';
    uint16_t offset_sum = 0;
    uint16_t offset_count = 0;
    uint16_t letter_calc = 0;

    pinyin_ime->dict = dict;

    for(uint16_t i = 0; ; i++) {
        if((NULL == (dict[i].py)) || (NULL == (dict[i].py_mb))) {
            headletter = dict[i - 1].py[0];
            letter_calc = headletter - 'a';
            pinyin_ime->py_num[letter_calc] = offset_count;
            break;
        }

        if(headletter == (dict[i].py[0])) {
            offset_count++;
        }
        else {
            headletter = dict[i].py[0];
            pinyin_ime->py_num[letter_calc] = offset_count;
            letter_calc = headletter - 'a';
            offset_sum += offset_count;
            pinyin_ime->py_pos[letter_calc] = offset_sum;

            offset_count = 1;
        }
    }
}

static char * pinyin_search_matching(lv_obj_t * obj, char * py_str, uint16_t * cand_num)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    const lv_pinyin_dict_t * cpHZ;
    uint8_t index, len = 0, offset;
    volatile uint8_t count = 0;

    if(*py_str == '\0')    return NULL;
    if(*py_str == 'i')     return NULL;
    if(*py_str == 'u')     return NULL;
    if(*py_str == 'v')     return NULL;
    if(*py_str == ' ')     return NULL;

    offset = py_str[0] - 'a';
    len = lv_strlen(py_str);

    cpHZ  = &pinyin_ime->dict[pinyin_ime->py_pos[offset]];
    count = pinyin_ime->py_num[offset];

    while(count--) {
        for(index = 0; index < len; index++) {
            if(*(py_str + index) != *((cpHZ->py) + index)) {
                break;
            }
        }

        // perfect match
        if(len == 1 || index == len) {
            // The Chinese character in UTF-8 encoding format is 3 bytes
            * cand_num = lv_strlen((const char *)(cpHZ->py_mb)) / 3;
            return (char *)(cpHZ->py_mb);
        }
        cpHZ++;
    }
    return NULL;
}

static void pinyin_ime_clear_data(lv_obj_t * obj)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

#if LV_IME_PINYIN_USE_K9_MODE
    if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
        pinyin_ime->k9_input_str_len = 0;
        pinyin_ime->k9_py_ll_pos = 0;
        pinyin_ime->k9_legal_py_count = 0;
        lv_memzero(pinyin_ime->k9_input_str,  LV_IME_PINYIN_K9_MAX_INPUT);
        lv_memzero(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        for(uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++) {
            lv_strcpy(lv_pinyin_k9_cand_str[i], " ");
        }
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT"\0");
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");
        lv_buttonmatrix_set_map(pinyin_ime->kb, (const char **)lv_btnm_def_pinyin_k9_map);
    }
#endif

    pinyin_ime->ta_count = 0;
    for(uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++) {
        lv_memset(lv_pinyin_cand_str[i], 0x00, sizeof(lv_pinyin_cand_str[i]));
        lv_pinyin_cand_str[i][0] = ' ';
    }
    lv_memzero(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));

    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
}

#if LV_IME_PINYIN_USE_K9_MODE
static void pinyin_k9_init_data(lv_obj_t * obj)
{
    LV_UNUSED(obj);

    uint16_t py_str_i = 0;
    uint16_t btnm_i = 0;
    for(btnm_i = 19; btnm_i < (LV_IME_PINYIN_K9_CAND_TEXT_NUM + 21); btnm_i++) {
        if(py_str_i == LV_IME_PINYIN_K9_CAND_TEXT_NUM) {
            lv_strcpy(lv_pinyin_k9_cand_str[py_str_i], LV_SYMBOL_RIGHT"\0");
        }
        else if(py_str_i == LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1) {
            lv_strcpy(lv_pinyin_k9_cand_str[py_str_i], "\0");
        }
        else {
            lv_strcpy(lv_pinyin_k9_cand_str[py_str_i], " \0");
        }

        lv_btnm_def_pinyin_k9_map[btnm_i] = lv_pinyin_k9_cand_str[py_str_i];
        py_str_i++;
    }

    default_kb_ctrl_k9_map[0]  = LV_BUTTONMATRIX_CTRL_CHECKED | 1;
    default_kb_ctrl_k9_map[1]  = LV_BUTTONMATRIX_CTRL_NO_REPEAT | LV_BUTTONMATRIX_CTRL_CLICK_TRIG | 1;
    default_kb_ctrl_k9_map[4]  = LV_BUTTONMATRIX_CTRL_CHECKED | 1;
    default_kb_ctrl_k9_map[5]  = LV_BUTTONMATRIX_CTRL_CHECKED | 1;
    default_kb_ctrl_k9_map[9]  = LV_KEYBOARD_CTRL_BUTTON_FLAGS | 1;
    default_kb_ctrl_k9_map[10] = LV_BUTTONMATRIX_CTRL_CHECKED | 1;
    default_kb_ctrl_k9_map[14] = LV_BUTTONMATRIX_CTRL_CHECKED | 1;
    default_kb_ctrl_k9_map[15] = LV_BUTTONMATRIX_CTRL_CHECKED | 1;
    default_kb_ctrl_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 16] = LV_BUTTONMATRIX_CTRL_CHECKED | 1;
}

static void pinyin_k9_get_legal_py(lv_obj_t * obj, char * k9_input, const char * py9_map[])
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    uint16_t len = lv_strlen(k9_input);

    if((len == 0) || (len >= LV_IME_PINYIN_K9_MAX_INPUT)) {
        return;
    }

    char py_comp[LV_IME_PINYIN_K9_MAX_INPUT] = {0};
    int mark[LV_IME_PINYIN_K9_MAX_INPUT] = {0};
    int index = 0;
    int flag = 0;
    uint16_t count = 0;

    uint32_t ll_len = 0;
    ime_pinyin_k9_py_str_t * ll_index = NULL;

    ll_len = lv_ll_get_len(&pinyin_ime->k9_legal_py_ll);
    ll_index = lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);

    while(index != -1) {
        if(index == len) {
            if(pinyin_k9_is_valid_py(obj, py_comp)) {
                if((count >= ll_len) || (ll_len == 0)) {
                    ll_index = lv_ll_ins_tail(&pinyin_ime->k9_legal_py_ll);
                    lv_strcpy(ll_index->py_str, py_comp);
                }
                else if((count < ll_len)) {
                    lv_strcpy(ll_index->py_str, py_comp);
                    ll_index = lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index);
                }
                count++;
            }
            index--;
        }
        else {
            flag = mark[index];
            if((size_t)flag < lv_strlen(py9_map[k9_input[index] - '2'])) {
                py_comp[index] = py9_map[k9_input[index] - '2'][flag];
                mark[index] = mark[index] + 1;
                index++;
            }
            else {
                mark[index] = 0;
                index--;
            }
        }
    }

    if(count > 0) {
        pinyin_ime->ta_count++;
        pinyin_ime->k9_legal_py_count = count;
    }
}

/*true: visible; false: not visible*/
static bool pinyin_k9_is_valid_py(lv_obj_t * obj, char * py_str)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    const lv_pinyin_dict_t * cpHZ = NULL;
    uint8_t index = 0, len = 0, offset = 0;
    volatile uint8_t count = 0;

    if(*py_str == '\0')    return false;
    if(*py_str == 'i')     return false;
    if(*py_str == 'u')     return false;
    if(*py_str == 'v')     return false;

    offset = py_str[0] - 'a';
    len = lv_strlen(py_str);

    cpHZ  = &pinyin_ime->dict[pinyin_ime->py_pos[offset]];
    count = pinyin_ime->py_num[offset];

    while(count--) {
        for(index = 0; index < len; index++) {
            if(*(py_str + index) != *((cpHZ->py) + index)) {
                break;
            }
        }

        // perfect match
        if(len == 1 || index == len) {
            return true;
        }
        cpHZ++;
    }
    return false;
}

static void pinyin_k9_fill_cand(lv_obj_t * obj)
{
    uint16_t index = 0, tmp_len = 0;
    ime_pinyin_k9_py_str_t * ll_index = NULL;

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    tmp_len = pinyin_ime->k9_legal_py_count;

    if(tmp_len != cand_len) {
        lv_memzero(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT"\0");
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");
        cand_len = tmp_len;
    }

    ll_index = lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);
    lv_strcpy(pinyin_ime->input_char, ll_index->py_str);

    for(uint8_t i = 0; i < LV_IME_PINYIN_K9_CAND_TEXT_NUM; i++) {
        lv_strcpy(lv_pinyin_k9_cand_str[i], " ");
    }

    while(ll_index) {
        if(index >= LV_IME_PINYIN_K9_CAND_TEXT_NUM)
            break;

        if(index < pinyin_ime->k9_legal_py_count) {
            lv_strcpy(lv_pinyin_k9_cand_str[index], ll_index->py_str);
        }

        ll_index = lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
        index++;
    }
    pinyin_ime->k9_py_ll_pos = index;

    lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);
    for(index = 0; index < pinyin_ime->k9_input_str_len; index++) {
        lv_textarea_delete_char(ta);
    }
    pinyin_ime->k9_input_str_len = lv_strlen(pinyin_ime->input_char);
    lv_textarea_add_text(ta, pinyin_ime->input_char);
}

static void pinyin_k9_cand_page_proc(lv_obj_t * obj, uint16_t dir)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);
    uint16_t ll_len =  lv_ll_get_len(&pinyin_ime->k9_legal_py_ll);

    if((ll_len > LV_IME_PINYIN_K9_CAND_TEXT_NUM) && (pinyin_ime->k9_legal_py_count > LV_IME_PINYIN_K9_CAND_TEXT_NUM)) {
        ime_pinyin_k9_py_str_t * ll_index = NULL;
        int count = 0;

        ll_index = lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);
        while(ll_index) {
            if(count >= pinyin_ime->k9_py_ll_pos)   break;

            ll_index = lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
            count++;
        }

        if((NULL == ll_index) && (dir == 1))   return;

        lv_memzero(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT"\0");
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");

        // next page
        if(dir == 1) {
            for(uint8_t i = 0; i < LV_IME_PINYIN_K9_CAND_TEXT_NUM; i++) {
                lv_strcpy(lv_pinyin_k9_cand_str[i], " ");
            }

            count = 0;
            while(ll_index) {
                if(count >= (LV_IME_PINYIN_K9_CAND_TEXT_NUM - 1))
                    break;

                lv_strcpy(lv_pinyin_k9_cand_str[count], ll_index->py_str);
                ll_index = lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
                count++;
            }
            pinyin_ime->k9_py_ll_pos += count - 1;

        }
        // previous page
        else {
            for(uint8_t i = 0; i < LV_IME_PINYIN_K9_CAND_TEXT_NUM; i++) {
                lv_strcpy(lv_pinyin_k9_cand_str[i], " ");
            }
            count = LV_IME_PINYIN_K9_CAND_TEXT_NUM - 1;
            ll_index = lv_ll_get_prev(&pinyin_ime->k9_legal_py_ll, ll_index);
            while(ll_index) {
                if(count < 0)  break;

                lv_strcpy(lv_pinyin_k9_cand_str[count], ll_index->py_str);
                ll_index = lv_ll_get_prev(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the previous list*/
                count--;
            }

            if(pinyin_ime->k9_py_ll_pos > LV_IME_PINYIN_K9_CAND_TEXT_NUM)
                pinyin_ime->k9_py_ll_pos -= 1;
        }

        lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
    }
}

#endif  /*LV_IME_PINYIN_USE_K9_MODE*/

#endif  /*LV_USE_IME_PINYIN*/
