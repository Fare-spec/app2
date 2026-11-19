#include <regex.h>
#include <stdio.h>
#include <string.h>

#include "flow.h"

static const char *patterns[] = {
    "Finished in (.*)$",
    "^\\.\\.\\. message ([0-9]+)\\.\\.\\. (.*)$",
    "^No not here, try (.*)$",
    "^That's better, what about (.*)$",
    "^You're close, don't give up: (.*)$",
    "^Nearly there, keep going: (.*)$",
    "^Almost there! (.*)$",
    "^No I'm joking, actually you are still pretty far from the cake (.*)$",
    "^Still trying\\? (.*)$",
    "^You know there is no end, right\\? (.*)$",
    "^Why are you still there\\? (.*)$",
    "^You do realize there is no cake, don't you\\? (.*)$",
    "^I told you, there is NO cake! (.*)$",
    "^Stop trying! (.*)$",
    "^I said: STOP TRYING (.*)$",
    "^You really never give up, do you\\? (.*)$",
    "^OK that's enough (.*)$",
    "^You think yourself smart, wiseass\\? (.*)$",
    "^Why don't you go to the other side of the planet AND GET LOST\\? (.*)$",
    "^Please\\.\\.\\. (.*)$",
    "^I said, PLEASE (.*)$",
    "^What do you not understand about this word\\? (.*)$",
    "^You like that, teasing me like you do\\? (.*)$",
    "^Let us see how much YOU can endure (.*)$",
    "^You will see how it feels\\.\\.\\. (.*)$",
    "^I don't care anymore (.*)$",
    "^You've reached message 25, congratulations\\.\\.\\. (.*)$",
    "^Now it's 26, hehehe (.*)$",
    "^It messes everything up doesn't it \\? \\(message 27\\) (.*)$",
    "^And it still goes on \\(message 28\\) (.*)$",
    "^Watch out, the cake is on page (.*), be careful!",
    "^You find a stepped-on cake, no need to continue there (.*)$",
    "^(.*) Do you understand why\\?$",
    "^Go to page (.*)!$",
    "^The cake is not there! Try page (.*)\\.$",
    "^The Cake seems far away! Check page (.*)\\.$",
    "^Page (.*) is probably where the cake is\\.\\.\\.!$",
    "^It's not there\\. Maybe the next pages will help\\? Next page is (.*)\\.$",
    "^A very small page (.*) for the road!$",
    "^See you on page (.*), if you're still on\\.\\.\\.$",
    "^You arrive somewhere\\.\\.\\. the only way out is through page (.*)\\. Will you\\.\\.\\.$",
    "^You see a road sign\\. It is written \"page\" on it\\.\nLooking carefully, you read \"Page page (.*)\"\\.",
    "^You arrive somewhere\\.\\.\\. You don't know where, but the only way out is through page (.*)\\. Will you go there or do a U-turn\\?$",
    "^(.*)\\.\\.\\.$",
    "^You are a courageous zealot searching a cake page (.*), and I am the villain\\.$",
    "^But no! You just keep going! (.*)$",
    "^I actually like that, that dedication\\.\\.\\. (.*)$",
    "^I call this game 'Where the heck is the page number\\?' (.*)$",
    "^I want to play a game (.*)$",
    "^It's an adventure game! (.*)$",
    "^Let us spice things up a little bit, will you\\? (.*)$",
    "^Let's go (.*)$",
    "^Owh boy, one hundred messages and you are still there! I'm amazed, seriously (.*)$",
    "^Pretending to believe in a cake that doesn't even exist (.*)$",
    "^Read a book or spend more time with your family and friends (.*)$",
    "^Think of all you could have done in the meantime\\.\\.\\. (.*)$",
    "^You REALLY don't EVER GIVE UP\\? (.*)$",
    "^♩ ♪ ♫ ♪♫ ♩♩♩ ♪ ♫ ♩♪♩♪ ♫ \\(whistling\\) (.*)$",
    "^You know what\\? I changed my mind (.*)$",
    "^I think in the end you deserve some cake (.*)$",
    "^But will you really be satisfied with it\\? (.*)$",
    "^Is that a real goal in life\\? (.*)$",
    "^What happens next\\? (.*)$",
    "^Another cake\\? Another lie\\? (.*)$",
    "^Another life\\? (.*)$",
    "^Or only a half-life\\?\\.\\.\\. (.*)$",
    "^What will remain of this journey\\? (.*)$",
    "^Some crumbs on the table\\.\\.\\. (.*)$"
};

static regex_t compiled_patterns[PATTERN_COUNT];
static bool patterns_ready;

bool gateau_flow_prepare(void) {
  if (patterns_ready) {
    return true;
  }
  for (size_t index = 0; index < PATTERN_COUNT; index++) {
    if (regcomp(&compiled_patterns[index], patterns[index], REG_EXTENDED) != 0) {
      while (index > 0) {
        regfree(&compiled_patterns[--index]);
      }
      return false;
    }
  }
  patterns_ready = true;
  return true;
}

bool gateau_flow_extract_key(const char *response, char *key, size_t capacity) {
  if (!gateau_flow_prepare()) {
    return false;
  }
  char text[MAX_FLOW_RESPONSE];
  snprintf(text, sizeof text, "%s", response);
  size_t length = strlen(text);
  while (length > 0 && text[length - 1] == '\n') {
    text[--length] = '\0';
  }

  for (size_t index = 0; index < PATTERN_COUNT; index++) {
    regmatch_t match[3];
    int result = regexec(&compiled_patterns[index], text, 3, match, 0);
    if (result != 0) {
      continue;
    }

    if (index == 0) {
      snprintf(key, capacity, "GlaDOS");
      return true;
    }

    size_t group = index == 1 ? 2 : 1;
    regoff_t start = match[group].rm_so;
    regoff_t end = match[group].rm_eo;
    if (start < 0 || end < start || (size_t)(end - start) >= capacity) {
      return false;
    }
    memcpy(key, text + start, (size_t)(end - start));
    key[end - start] = '\0';
    return true;
  }

  return false;
}
